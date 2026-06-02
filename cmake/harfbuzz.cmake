function(provide_harfbuzz HB_VERSION)
    set(HB_BUILD_SUBSET OFF)
    set(HB_BUILD_RASTER OFF)
    set(HB_BUILD_GPU OFF)
    set(HB_BUILD_VECTOR OFF)

	include(FetchContent)
    FetchContent_Declare(
        harfbuzz-s
        GIT_REPOSITORY https://github.com/harfbuzz/harfbuzz.git
        GIT_TAG ${HB_VERSION}
    )

    FetchContent_MakeAvailable(harfbuzz-s)
endfunction()