{
  inputs.nixpkgs.url = "github:NixOS/nixpkgs/nixpkgs-unstable";

  outputs =
    {
      self,
      nixpkgs,
    }:
    let
      supportedSystems = [
        "x86_64-linux"
        "aarch64-linux"
      ];

      forAllSystems = f: nixpkgs.lib.genAttrs supportedSystems (system: f system);
      pkgsForSystem =
        system:
        import nixpkgs {
          inherit system;
          config.allowUnfree = true; # probably unneeded
        };
    in
    {
      devShells = forAllSystems (
        system:
        let
          pkgs = pkgsForSystem system;
          commonPackages = [
            (
              if pkgs ? gcc-arm-embedded then
                pkgs.gcc-arm-embedded
              else
                pkgs.pkgsCross.arm-embedded.buildPackages.gcc
            )
            pkgs.qemu
            pkgs.bear
            pkgs.lrzsz
            pkgs.util-linux
            pkgs.clang-tools
            pkgs.minicom
            pkgs.parted
          ];

          debuggerPackage =
            if pkgs ? seer then
              [ pkgs.seer ]
            else if pkgs ? gdb then
              [ pkgs.gdb ]
            else
              [ ];
        in
        {
          default = pkgs.mkShell {
            buildInputs = commonPackages ++ debuggerPackage;
          };
        }
      );
    };
}
