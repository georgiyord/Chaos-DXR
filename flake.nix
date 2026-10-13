{
  description = "Working environment for developing DX12-DXR application on linux with wine and vkd3d-proton";

  inputs = {
    nixpkgs.url = "https://channels.nixos.org/nixpkgs-unstable/nixexprs.tar.zst";
  };

  outputs =
    inputs:
    let
      system = "x86_64-linux";
      pkgs = import inputs.nixpkgs { inherit system; };
    in
    {
      devShells.x86_64-linux.default = pkgs.mkShell {
        buildInputs = with pkgs; [
          wine64
          winetricks
          python3
          msitools
          cmake
          cmake-language-server
          cmake-format
          ninja
        ];
        shellHook = ''
          source ./tools/wine-msvc/wine-env.sh
          ./tools/wine-msvc/init.sh --skip-test --skip-fetch
          # export PATH="$(realpath ./MSVC/bin/x64):$PATH"
        '';
      };
    };
}
