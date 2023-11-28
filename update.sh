#!/bin/bash

# quick script to update all dependencies
# 3-tuple of git repo, ref, and patterns of unneeded files to remove to save space
# if a patch file exists with the same name as the repo, it is applied on update

REPOS=(
    "https://github.com/libsdl-org/SDL.git" "release-2.0.22" "test"
    "https://github.com/libsdl-org/SDL_image.git" "release-2.6.1" "external/libjxl external/dav1d external/libtiff external/libwebp external/libavif"
    "https://github.com/libsdl-org/SDL_mixer.git" "release-2.6.1" "external/flac external/mpg123 exernal/opus"
    "https://github.com/xiph/theora.git" "v1.1.1" ""
    "https://github.com/LuaDist/bzip2.git" "1.0.5" "manual.ps manual.pdf"
    "https://github.com/LuaDist/lcms.git" "1.19" "python "
    "https://github.com/LuaDist/libmng.git" "1.0.10" ""
    "https://github.com/psi29a/unsf.git" "c5e5a8c" ""
    "https://github.com/doctest/doctest.git" "v2.4.11" ""
    "https://github.com/mjansson/mdns" "1.1" ""
    "https://github.com/oo13/spiritless_po" "e2d9835d061c6f660bcd8afb90ea5b8b981ba4a5" ""
    "https://github.com/wheybags/guichan" "98bb8a588de6c2556c158f13a7e807070176c9a2" "" # 0.4.0
)

for i in `seq 0 3 ${#REPOS[@]}`; do
    repo=${REPOS[$i]}
    ref=${REPOS[$((i+1))]}
    del_pattern=${REPOS[$((i+2))]}
    dir=${repo%.git}
    dir=${dir##*/}

    if [ -n "$repo" ]; then
        echo "Cloning $repo at $ref to $dir"
        set -xe
        git rm -rf "$dir" || rm -rf "$dir" || true
        git clone --depth 1 --branch "$ref" --single-branch "$repo" "$dir" || ( git clone "$repo" "$dir" && git -C "$dir" checkout "$ref" )
        git -C "$dir" submodule update --init --recursive --depth 1
        rm -rf "${dir}/.git"
        if [ -n "$del_pattern" ]; then
            cd "$dir"
            rm -rf $del_pattern
            cd ..
        fi
        if [ -e "${dir}.patch" ]; then
            patch -p1 < "${dir}.patch"
        fi
        git add "$dir"
        git commit -m "Inline $repo at $ref"
        set +xe
    fi
done
