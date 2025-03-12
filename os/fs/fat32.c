#include "fat32.h"
#include <stdbool.h>

/* TODO: when mmc driver is implemented, export symbol to struct fat32_diskio_t within the driver and pass it to
 * fat32_mount.
 *  TODO: move lib fns to common lib when linked
 */

char* strchr(const char* str, int ch)
{
    while (*str != '\0')
    {
        if (*str == ch)
        {
            return (char*) str;
        }
        str++;
    }
    return (ch == '\0') ? (char*) str : NULL;
}

size_t strlen(const char* str)
{
    const char* s = str;
    while (*s)
        s++;
    return s - str;
}

int toupper(int c)
{
    if (c >= 'a' && c <= 'z')
    {
        return c - ('a' - 'A');
    }
    return c;
}

int memcmp(const void* s1, const void* s2, size_t n)
{
    const unsigned char *p1 = s1, *p2 = s2;
    size_t i;
    for (i = 0; i < n; i++)
    {
        if (p1[i] != p2[i])
        {
            return p1[i] - p2[i];
        }
    }
    return 0;
}

char* strncpy(char* dest, const char* src, size_t n)
{
    size_t i;
    for (i = 0; i < n && src[i] != '\0'; i++)
    {
        dest[i] = src[i];
    }
    for (; i < n; i++)
    {
        dest[i] = '\0';
    }
    return dest;
}

void* memcpy(void* dest, const void* src, size_t n)
{
    const char* s = src;
    char* d       = dest;
    size_t i;
    for (i = 0; i < n; i++)
    {
        d[i] = s[i];
    }
    return dest;
}

#define MIN(a, b) ((a) < (b) ? (a) : (b))

/* static helpers */
static void parse_fat32_path(const char* path, fat32_path_t* parser);
static int read_dir_entry(fat32_fs_t* fs, fat32_dir_entry_t* current_dir, const char* name);
static int get_cluster_at_index(fat32_fs_t* fs, uint32_t start_cluster, uint32_t index, uint32_t* target_cluster);
static int fat32_read_fat_entry(fat32_fs_t* fs, uint32_t cluster);
static uint32_t fat32_get_next_cluster(fat32_fs_t* fs, uint32_t curr);
static int32_t cluster_to_sector(fat32_fs_t* fs, uint32_t cluster);
static void format_name_to_fat32(const char* input, char* output);

static int find_partition_via_mbr(const MBR* mbr, uint32_t* start_sector);
static int check_sector0_for_fat32(const uint8_t* sector_buffer, uint32_t* start_sector);
static int validate_fat32_boot_sector(const Fat32BootSector* boot_sector);
static void initialize_fat32_fs(fat32_fs_t* fs,
                                const fat32_diskio_t* io,
                                const Fat32BootSector* boot_sector,
                                uint32_t fat32_start_sector);

/*                     */
/* fat32 api functions */
/*                     */
int fat32_mount(fat32_fs_t* fs, const fat32_diskio_t* io)
{
    MBR* mbr;
    Fat32BootSector* boot_sector;
    uint8_t sector_buffer[FAT32_SECTOR_SIZE];
    uint32_t fat32_start_sector = 0;
    bool found_via_mbr          = false;
    bool found_via_sector0      = false;

    /* Parameter validation */
    if (!fs || !io || !io->read_sector)
    {
        return FAT32_ERROR_BAD_PARAMETER;
    }

    /* Read sector 0 (MBR or boot sector) */
    if (io->read_sector(0, sector_buffer) != 0)
    {
        return FAT32_ERROR_IO;
    }

    /* Check for MBR with valid FAT32 partition */
    mbr = (MBR*) sector_buffer;
    if (mbr->signature == MBR_SIGNATURE)
    {
        found_via_mbr = find_partition_via_mbr(mbr, &fat32_start_sector);
    }

    /* Fallback to check sector 0 directly if no MBR partition found */
    if (!found_via_mbr)
    {
        found_via_sector0 = check_sector0_for_fat32(sector_buffer, &fat32_start_sector);
    }

    /* Final validation check */
    if (!found_via_mbr && !found_via_sector0)
    {
        return FAT32_ERROR_INVALID_BOOT_SECTOR;
    }

    /* Read actual boot sector if we found via MBR */
    if (found_via_mbr)
    {
        if (io->read_sector(fat32_start_sector, sector_buffer) != 0)
        {
            return FAT32_ERROR_IO;
        }
    }

    /* Validate boot sector structure */
    boot_sector = (Fat32BootSector*) sector_buffer;
    if (validate_fat32_boot_sector(boot_sector) != 0)
    {
        return FAT32_ERROR_INVALID_BOOT_SECTOR;
    }

    /* Initialize filesystem structure */
    initialize_fat32_fs(fs, io, boot_sector, fat32_start_sector);
    return FAT32_SUCCESS;
}

int fat32_open(fat32_fs_t* fs, const char* path, fat32_file_t* file)
{
    const char* target_name;
    uint32_t i, parent_cluster;
    fat32_dir_entry_t current_dir;
    fat32_path_t path_struct;

    if (!fs || !fat32_is_initialized(fs) || !path || !file)
    {
        return FAT32_ERROR_BAD_PARAMETER;
    }

    fat32_path_init(&path_struct);
    parse_fat32_path(path, &path_struct);

    parent_cluster             = fs->root_cluster; /* Start from root as parent */
    current_dir.is_initialized = false;
    for (i = 0; i < (uint32_t) path_struct.num_components - 1; i++)
    {
        /* Traverse directories to find the parent */
        if (read_dir_entry(fs, &current_dir, path_struct.components[i]) != 0)
        {
            return FAT32_ERROR_NO_PATH;
        }
        parent_cluster = current_dir.start_cluster;
    }

    /* Open the target file/dir */
    target_name = path_struct.components[path_struct.num_components - 1];
    if (read_dir_entry(fs, &current_dir, target_name) != 0)
    {
        return FAT32_ERROR_NO_FILE;
    }

    if ((current_dir.attributes & FAT32_ATTR_DIRECTORY) == FAT32_ATTR_DIRECTORY)
    {
        return FAT32_ERROR_IS_DIR;
    }

    file->fs                 = fs;
    file->start_cluster      = current_dir.start_cluster;
    file->current_cluster    = current_dir.start_cluster;
    file->file_size          = current_dir.file_size;
    file->parent_dir_cluster = parent_cluster;
    file->file_offset        = 0;
    format_name_to_fat32(target_name, file->formatted_name);

    return FAT32_SUCCESS;
}

int fat32_read(fat32_file_t* file, void* buffer, int size)
{
    fat32_fs_t* fs;
    int32_t cluster_result;
    uint8_t sectors_per_cluster;
    uint32_t cluster_size;
    uint32_t bytes_read = 0;
    uint8_t* buf_ptr    = (uint8_t*) buffer;
    uint32_t remaining  = size;
    uint32_t max_readable;

    if (!file || !buffer)
    {
        return FAT32_ERROR_BAD_PARAMETER;
    }

    if (file->file_offset >= file->file_size || size == 0)
    {
        return 0;
    }

    fs                  = file->fs;
    sectors_per_cluster = fs->sectors_per_cluster;
    cluster_size        = fs->bytes_per_sector * sectors_per_cluster;

    /* Calculate the maximum readable bytes */
    max_readable = file->file_size - file->file_offset;
    if (remaining > max_readable)
    {
        remaining = max_readable;
    }

    while (remaining > 0)
    {
        uint32_t bytes_in_cluster, bytes_to_read, cluster_start_sector, sector_offset;
        uint32_t sector_in_cluster;
        uint32_t current_sector;

        uint32_t cluster_offset = file->file_offset % cluster_size;
        uint32_t cluster_index  = file->file_offset / cluster_size;
        uint32_t target_cluster;

        int result = get_cluster_at_index(fs, file->start_cluster, cluster_index, &target_cluster);
        if (result != FAT32_SUCCESS)
        {
            return bytes_read > 0 ? (int) bytes_read : result;
        }

        /* Calculate how much we can read from this cluster */
        bytes_in_cluster = cluster_size - cluster_offset;
        bytes_to_read    = (remaining < bytes_in_cluster) ? remaining : bytes_in_cluster;

        /* Calculate starting sector and position within the cluster */
        cluster_result = cluster_to_sector(fs, target_cluster);
        if (cluster_result < 0)
            return cluster_result;
        cluster_start_sector = (uint32_t) cluster_result;

        sector_offset     = cluster_offset % fs->bytes_per_sector;
        sector_in_cluster = cluster_offset / fs->bytes_per_sector;
        current_sector    = cluster_start_sector + sector_in_cluster;

        /* Read sectors within the cluster until we fulfill the request or exhaust the cluster */
        while (bytes_to_read > 0)
        {
            volatile uint8_t* src;
            uint32_t bytes_in_sector, bytes_from_sector;
            size_t i;
            uint8_t sector_buffer[FAT32_SECTOR_SIZE] __attribute__((aligned(8))); /* 8byte alignment for arm */
            result = fs->disk.read_sector(current_sector, sector_buffer);
            if (result != 0)
            {
                return bytes_read > 0 ? (int) bytes_read : FAT32_ERROR_IO;
            }

            bytes_in_sector   = fs->bytes_per_sector - sector_offset;
            bytes_from_sector = (bytes_to_read < bytes_in_sector) ? bytes_to_read : bytes_in_sector;

            src = sector_buffer + sector_offset;
            for (i = 0; i < bytes_from_sector; i++)
            {
                buf_ptr[i] = src[i];
            }
            buf_ptr += bytes_from_sector;
            bytes_read += bytes_from_sector;
            remaining -= bytes_from_sector;
            file->file_offset += bytes_from_sector;
            bytes_to_read -= bytes_from_sector;

            /* Move to next sector and reset offset */
            current_sector++;
            sector_offset = 0;

            /* Check if we've exceeded the current cluster */
            if ((current_sector - cluster_start_sector) >= sectors_per_cluster)
            {
                break;
            }
        }

        /* Update current_cluster to the last accessed cluster */
        file->current_cluster = target_cluster;
        /* If more data is needed, move to the next cluster */
        if (bytes_to_read > 0)
        {
            uint32_t next_cluster = fat32_get_next_cluster(fs, target_cluster);
            if (next_cluster >= FAT32_LAST_MARKER || next_cluster == FAT32_EOC_MARKER)
            {
                break;
            }
        }
    }

    return bytes_read;
}

/* Function to read the volume label from the root directory */
int fat32_readlabel(fat32_fs_t* fs, char* label_out)
{
    uint8_t sector_buffer[FAT32_SECTOR_SIZE];
    uint32_t current_cluster, sector_target;
    int next_cluster;
    Fat32DirectoryEntry* dir_entry;

    current_cluster = fs->root_cluster;
    sector_target   = fs->first_data_sector + (current_cluster - 2) * fs->sectors_per_cluster;
    dir_entry       = (Fat32DirectoryEntry*) sector_buffer;

    while (current_cluster != FAT32_EOC_MARKER)
    {
        uint32_t sector;
        for (sector = 0; sector < fs->sectors_per_cluster; sector++)
        {
            uint32_t entry;
            int ret = fs->disk.read_sector(sector_target + sector, sector_buffer);
            if (ret != 0)
            {
                return FAT32_ERROR_IO;
            }

            /* Process all directory entries in this sector */
            for (entry = 0; entry < FAT32_SECTOR_SIZE / sizeof(Fat32DirectoryEntry); entry++)
            {
                Fat32DirectoryEntry* current_entry = &dir_entry[entry];

                if (current_entry->filename[0] == FAT32_ENTRY_EMPTY)
                {
                    return FAT32_ERROR_NO_LABEL; /* EOD, no label found */
                }
                if (current_entry->filename[0] == FAT32_ENTRY_DELETED)
                {
                    continue;
                }

                if (current_entry->attr & FAT32_ATTR_VOLLABEL)
                {
                    memcpy(label_out, current_entry->filename, 11);
                    label_out[11] = '\0';
                    return FAT32_SUCCESS;
                }
            }
        }

        /* Read next cluster from FAT */
        next_cluster = fat32_read_fat_entry(fs, current_cluster);
        if (next_cluster < 0)
        {
            return next_cluster; /* Error reading FAT */
        }

        current_cluster = next_cluster;
        sector_target   = fs->first_data_sector + (current_cluster - 2) * fs->sectors_per_cluster;
    }
    return FAT32_ERROR_NO_LABEL;
}

int fat32_tell(fat32_file_t* file)
{
    if (!file)
    {
        return FAT32_ERROR_BAD_PARAMETER;
    }

    return file->file_offset;
}

int fat32_seek(fat32_file_t* file, int offset)
{
    if (!file)
    {
        return FAT32_ERROR_BAD_PARAMETER;
    }

    if (offset < 0 || (uint32_t) offset > file->file_size)
    {
        return FAT32_ERROR_BAD_PARAMETER;
    }

    file->file_offset = offset;
    return FAT32_SUCCESS;
}

int fat32_eof(fat32_file_t* file)
{
    if (!file)
    {
        return FAT32_ERROR_BAD_PARAMETER;
    }

    return file->file_offset >= file->file_size;
}

int fat32_size(fat32_file_t* file)
{
    if (!file)
    {
        return FAT32_ERROR_BAD_PARAMETER;
    }

    return file->file_size;
}

int fat32_close(fat32_file_t* file)
{
    if (!file)
    {
        return FAT32_ERROR_BAD_PARAMETER;
    }

    return FAT32_SUCCESS;
}

static uint32_t fat32_get_next_cluster(fat32_fs_t* fs, uint32_t curr)
{
    uint8_t buffer[FAT32_SECTOR_SIZE];
    uint32_t entry, value, fat_offset, entry_offset, fat_sector;
    int ret;
    if (!fs)
    {
        return FAT32_ERROR_BAD_PARAMETER;
    }

    /* Each FAT entry is 4 bytes */
    fat_offset = curr * 4;

    /* Calculate which FAT sector contains this cluster */
    entry_offset = fat_offset % FAT32_SECTOR_SIZE;
    fat_sector   = fat_offset / FAT32_SECTOR_SIZE;
    fat_sector += fs->fat_start_sector;

    /* Read the FAT sector */
    ret = fs->disk.read_sector(fat_sector, buffer);
    if (ret != 0)
    {
        return FAT32_ERROR_IO;
    }

    /* Get the FAT entry value */
    entry = 0;
    entry |= buffer[entry_offset];
    entry |= buffer[entry_offset + 1] << 8;
    entry |= buffer[entry_offset + 2] << 16;
    entry |= buffer[entry_offset + 3] << 24;
    value = entry & 0x0FFFFFFF;

    /* Check for special cluster values */
    if (value >= 0x0FFFFFF8)
    {
        value = FAT32_EOC_MARKER;
    }
    else if (value == 0x0FFFFFF7)
    {
        return FAT32_ERROR_IO;
    }
    else if (value < 2)
    { /* Check for reserved clusters */
        return FAT32_ERROR_CORRUPTED_FS;
    }

    return value;
}

static int get_cluster_at_index(fat32_fs_t* fs, uint32_t start_cluster, uint32_t index, uint32_t* target_cluster)
{
    uint32_t i, current_cluster;

    if (start_cluster <= 2)
    {
        return FAT32_ERROR_BAD_PARAMETER;
    }

    current_cluster = start_cluster;
    for (i = 0; i < index; ++i)
    {
        uint32_t next_cluster = fat32_get_next_cluster(fs, current_cluster);

        /* Check if next_cluster is EOC prematurely */
        if (next_cluster == FAT32_EOC_MARKER)
        { /* Assuming is_eoc() checks for EOC range */
            if (i < index - 1)
            { /* Only error if not the last iteration */
                return FAT32_ERROR_CORRUPTED_FS;
            }
        }

        current_cluster = next_cluster;
    }

    *target_cluster = current_cluster;
    return FAT32_SUCCESS;
}

static int fat32_read_fat_entry(fat32_fs_t* fs, uint32_t cluster)
{
    int ret;
    uint32_t next_cluster, fat_offset, entry_offset, fat_sector;
    uint8_t buffer[FAT32_SECTOR_SIZE];
    if (!fs)
    {
        return FAT32_ERROR_BAD_PARAMETER;
    }

    /* Calculate which sector of the FAT contains this cluster's entry */
    /* Each FAT entry is 4 bytes (32 bits) in FAT32 */
    fat_offset   = cluster * 4;
    entry_offset = fat_offset % FAT32_SECTOR_SIZE;
    fat_sector   = fat_offset / FAT32_SECTOR_SIZE;
    fat_sector += fs->fat_start_sector;

    /* Read the FAT sector */

    ret = fs->disk.read_sector(fat_sector, buffer);
    if (ret != 0)
    {
        return FAT32_ERROR_IO;
    }

    /* Extract the 32-bit FAT entry */
    next_cluster = buffer[entry_offset];
    next_cluster &= 0x0FFFFFFF;

    /* Check for special values */
    if (next_cluster >= 0x0FFFFFF8)
    {
        next_cluster = FAT32_EOC_MARKER;
    }
    else if (next_cluster == 0x0FFFFFF7)
    {
        return FAT32_ERROR_IO;
    }

    return next_cluster;
}

static void format_name_to_fat32(const char* input, char* output)
{
    size_t name_len = 0;
    size_t ext_len  = 0;
    size_t i;
    const char* ext_pos = strchr(input, '.');

    if (ext_pos)
    {
        name_len = ext_pos - input;
        ext_len  = strlen(ext_pos + 1);
    }
    else
    {
        name_len = strlen(input);
        ext_len  = 0;
    }

    /* Copy name (up to 8 characters) */
    for (i = 0; i < 8; i++)
    {
        if (i < name_len)
        {
            output[i] = toupper((unsigned char) input[i]);
        }
        else
        {
            output[i] = ' ';
        }
    }

    /* Copy extension (up to 3 characters) */
    for (i = 0; i < 3; i++)
    {
        if (ext_pos && i < ext_len)
        {
            output[8 + i] = toupper(ext_pos[1 + i]);
        }
        else
        {
            output[8 + i] = ' ';
        }
    }
}

/* Function to parse a FAT32 path into components - STATIC */
static void parse_fat32_path(const char* path, fat32_path_t* parser)
{
    int path_len      = strlen(path);
    int start         = 0;
    int component_idx = 0;
    int i;

    /* skip leading dot if present */
    if (path[0] == '.')
    {
        ++path;
        --path_len;
    }

    for (i = 0; i <= path_len; ++i)
    {
        /* If we reach a separator or the end of the string, we capture the component */
        if (path[i] == '/' || path[i] == '\0')
        {
            if (i > start)
            {
                int len = i - start;
                if (len < FAT32_MAX_COMPONENT_LENGTH && component_idx < FAT32_MAX_COMPONENTS)
                {
                    strncpy(parser->components[component_idx], &path[start], len);
                    parser->components[component_idx][len] = '\0';
                    ++component_idx;
                }
            }
            start = i + 1; /* Set the start of the next component */
        }
    }
    parser->num_components = component_idx;
}

int read_dir_entry(fat32_fs_t* fs, fat32_dir_entry_t* current_dir, const char* name)
{
    Fat32DirectoryEntry* dir_entry;
    int next_cluster, i;
    uint8_t sector_buffer[FAT32_SECTOR_SIZE];
    uint32_t current_cluster, sector_target;
    char formatted_name[11];
    if (!fs || !name)
        return FAT32_ERROR_BAD_PARAMETER;

    /* If current_dir is NULL or not initialized, start from root directory */
    if (!current_dir || !current_dir->is_initialized)
    {
        current_cluster = fs->root_cluster;
        sector_target   = fs->first_data_sector + (current_cluster - 2) * fs->sectors_per_cluster;
    }
    else
    {
        current_cluster = current_dir->start_cluster;
        sector_target   = fs->first_data_sector + (current_cluster - 2) * fs->sectors_per_cluster;
    }

    dir_entry = (Fat32DirectoryEntry*) sector_buffer;
    for (i = 0; i < 11; i++)
    {
        formatted_name[i] = ' ';
    }
    format_name_to_fat32(name, formatted_name);
    while (current_cluster != FAT32_EOC_MARKER)
    {
        uint32_t sector;
        for (sector = 0; sector < fs->sectors_per_cluster; sector++)
        {
            uint32_t entry;
            int ret = fs->disk.read_sector(sector_target + sector, sector_buffer);
            if (ret != 0)
            {
                return FAT32_ERROR_IO;
            }

            /* Process all directory entries in this sector */
            for (entry = 0; entry < FAT32_SECTOR_SIZE / sizeof(Fat32DirectoryEntry); entry++)
            {
                Fat32DirectoryEntry* current_entry = &dir_entry[entry];
                if (current_entry->filename[0] == FAT32_ENTRY_EMPTY)
                {
                    return -1; /* TODO: Return proper graceful handling */
                }

                if (current_entry->filename[0] == FAT32_ENTRY_DELETED)
                {
                    continue;
                }

                if (memcmp(formatted_name, current_entry->filename, 11) == 0)
                {
                    /* Found the entry, update current_dir if provided */
                    if (current_dir)
                    {
                        current_dir->is_initialized = 1;
                        current_dir->start_cluster =
                            (current_entry->firstClusterHigh << 16) | current_entry->firstClusterLow;
                        current_dir->file_size  = current_entry->fileSize;
                        current_dir->attributes = current_entry->attr;
                        /*
                        // if (current_dir->start_cluster < 2) {
                        //     printk("Start clister: %d\n", current_dir->start_cluster);
                        //     return FAT32_ERROR_CORRUPTED_FS;
                        // }

                        // TODO other metadata crap we don't care about for now
                        */
                    }
                    return 0;
                }
            }
        }

        /* Read next cluster from FAT */
        next_cluster = fat32_read_fat_entry(fs, current_cluster);
        if (next_cluster < 0)
        {
            return next_cluster;
        }

        current_cluster = next_cluster;
        sector_target   = fs->first_data_sector + (current_cluster - 2) * fs->sectors_per_cluster;
    }

    return FAT32_ERROR_NO_FILE; /* Entry not found */
}

static int32_t cluster_to_sector(fat32_fs_t* fs, uint32_t cluster)
{
    if (cluster < 2 || cluster >= (2 + fs->total_clusters))
    {
        return FAT32_ERROR_INVALID_CLUSTER;
    }
    return ((cluster - 2) * fs->sectors_per_cluster) + fs->first_data_sector;
}

static int find_partition_via_mbr(const MBR* mbr, uint32_t* start_sector)
{
    int i;
    for (i = 0; i < 4; i++)
    {
        if (mbr->partitions[i].partition_type == FAT32_PART_TYPE_1 ||
            mbr->partitions[i].partition_type == FAT32_PART_TYPE_2)
        {
            *start_sector = mbr->partitions[i].start_sector;
            return true;
        }
    }
    return false;
}

static int check_sector0_for_fat32(const uint8_t* sector_buffer, uint32_t* start_sector)
{
    const Fat32BootSector* bs = (const Fat32BootSector*) sector_buffer;
    if (bs->bootSectorSig == FAT32_BOOT_SECTOR_SIGNATURE && bs->sectorsPerFAT32 != 0)
    {
        *start_sector = 0;
        return true;
    }
    return false;
}

static int validate_fat32_boot_sector(const Fat32BootSector* boot_sector)
{
    if (boot_sector->bootSectorSig != FAT32_BOOT_SECTOR_SIGNATURE || boot_sector->sectorsPerCluster == 0 ||
        boot_sector->reservedSectors == 0 || boot_sector->numFATs == 0 || boot_sector->sectorsPerFAT32 == 0)
    {
        return FAT32_ERROR_INVALID_BOOT_SECTOR;
    }
    return FAT32_SUCCESS;
}

static void initialize_fat32_fs(fat32_fs_t* fs,
                                const fat32_diskio_t* io,
                                const Fat32BootSector* boot_sector,
                                uint32_t fat32_start_sector)
{
    fs->disk                  = *io;
    fs->bytes_per_sector      = FAT32_SECTOR_SIZE;
    fs->sectors_per_cluster   = boot_sector->sectorsPerCluster;
    fs->reserved_sector_count = boot_sector->reservedSectors;
    fs->num_fats              = boot_sector->numFATs;
    fs->sectors_per_fat       = boot_sector->sectorsPerFAT32;
    fs->fat_start_sector      = fat32_start_sector + boot_sector->reservedSectors;
    fs->first_data_sector =
        fat32_start_sector + boot_sector->reservedSectors + (boot_sector->numFATs * boot_sector->sectorsPerFAT32);
    fs->root_cluster   = boot_sector->rootCluster;
    fs->cluster_size   = fs->sectors_per_cluster * fs->bytes_per_sector;
    fs->total_sectors  = (boot_sector->totalSectors16 != 0) ? boot_sector->totalSectors16 : boot_sector->totalSectors32;
    fs->total_clusters = (fs->total_sectors - (fs->first_data_sector - fat32_start_sector)) / fs->sectors_per_cluster;
    fs->magic          = FAT32_MAGIC;
}
