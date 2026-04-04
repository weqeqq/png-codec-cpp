{
  description = "wqpng: C++23 PNG codec library";

  inputs = {
    nixpkgs.url = "github:NixOS/nixpkgs/nixos-unstable";
    wqcolor = {
      url = "github:weqeqq/wqcolor";
      inputs.nixpkgs.follows = "nixpkgs";
    };
    wqparallel = {
      url = "github:weqeqq/wqparallel";
      inputs.nixpkgs.follows = "nixpkgs";
    };
    fpng = {
      url = "github:weqeqq/fpng";
      inputs.nixpkgs.follows = "nixpkgs";
    };
    wqfile = {
      url = "github:weqeqq/wqfile";
      inputs.nixpkgs.follows = "nixpkgs";
    };
    wuffs = {
      url = "github:weqeqq/wuffs";
      inputs.nixpkgs.follows = "nixpkgs";
    };
  };

  outputs =
    { nixpkgs, wqcolor, wqparallel, fpng, wqfile, wuffs, ... }:
    let
      systems = [
        "x86_64-linux"
        "aarch64-linux"
        "x86_64-darwin"
        "aarch64-darwin"
      ];

      forAllSystems = f: nixpkgs.lib.genAttrs systems f;

      perSystem =
        system:
        let
          pkgs = import nixpkgs { inherit system; };
          lib = pkgs.lib;

          src = lib.fileset.toSource {
            root = ./.;
            fileset = lib.fileset.unions [
              ./LICENSE
              ./headers
              ./meson.build
              ./meson_options.txt
              ./sources
              ./subprojects/.wraplock
              ./subprojects/fpng.wrap
              ./subprojects/gtest.wrap
              ./subprojects/lodepng.wrap
              (lib.fileset.maybeMissing ./subprojects/wqcolor.wrap)
              ./subprojects/wqfile.wrap
              ./subprojects/wuffs.wrap
              ./subprojects/packagefiles/lodepng-meson
              ./tests
            ];
          };

          wqcolorPkg = wqcolor.packages.${system}.default;
          wqparallelPkg = wqparallel.packages.${system}.default;
          fpngPkg = fpng.packages.${system}.default;
          wqfilePkg = wqfile.packages.${system}.default;
          wuffsPkg = wuffs.packages.${system}.default;

          mkWqpng =
            {
              tests ? false,
            }:
            pkgs.stdenv.mkDerivation {
              pname = "wqpng";
              version = "0.1.0";
              inherit src;

              strictDeps = true;

              nativeBuildInputs = with pkgs; [
                meson
                ninja
                pkg-config
              ];

              buildInputs =
                [
                  wqcolorPkg
                  wqparallelPkg
                  fpngPkg
                  wqfilePkg
                  wuffsPkg
                ]
                ++ lib.optionals tests [ pkgs.gtest ];

              mesonFlags = [
                "-Dinstall=true"
                "-Dtests=${if tests then "true" else "false"}"
              ];

              doCheck = tests;
              checkPhase = lib.optionalString tests ''
                meson test --print-errorlogs
              '';

              meta = with lib; {
                description = "C++23 PNG codec library built on top of wqcolor";
                homepage = "https://github.com/weqeqq/wqpng";
                license = licenses.mit;
                platforms = platforms.unix;
              };
            };

          package = mkWqpng { };
          checkPackage = mkWqpng { tests = true; };
        in
        {
          inherit pkgs package checkPackage;
        };
    in
    {
      packages = forAllSystems (
        system:
        let
          inherit (perSystem system) package;
        in
        {
          default = package;
          wqpng = package;
        }
      );

      checks = forAllSystems (
        system:
        let
          inherit (perSystem system) checkPackage;
        in
        {
          default = checkPackage;
          wqpng-tests = checkPackage;
        }
      );

      devShells = forAllSystems (
        system:
        let
          inherit (perSystem system) package pkgs;
        in
        {
          default = pkgs.mkShell {
            inputsFrom = [ package ];

            packages = with pkgs; [
              clang-tools
              cmake
              gtest
              nixpkgs-fmt
              python3
            ];
          };
        }
      );

      formatter = forAllSystems (system: (perSystem system).pkgs.nixpkgs-fmt);
    };
}
