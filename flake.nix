{
  description = "Flake for je2be-core";

  inputs = {
    nixpkgs.url = "github:NixOS/nixpkgs/nixos-26.05";
  };

  outputs =
    {
      self,
      nixpkgs,
    }:
    let
      supportedSystems = [ "x86_64-linux" ];
      forEachSupportedSystem =
        f:
        nixpkgs.lib.genAttrs supportedSystems (
          system:
          f {
            pkgs = import nixpkgs { inherit system; };
          }
        );
    in
    {
      devShells = forEachSupportedSystem (
        { pkgs }:
        {
          default =
            pkgs.mkShell.override
              {
                # project uses clang 18
                # 19 fails to build
                stdenv = pkgs.llvmPackages_18.libcxxStdenv;
              }
              {
                packages = with pkgs; [
                  ccache
                  cmake
                  ninja
                  zlib
                ];
                shellHook = ''
                  # Add CMake prefix path for Zlib
                  export CMAKE_PREFIX_PATH=$CMAKE_PREFIX_PATH:${pkgs.zlib}/lib/cmake
                '';
              };
        }
      );
    };
}
