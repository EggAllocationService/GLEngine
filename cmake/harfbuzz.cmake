function(provide_harfbuzz HB_VERSION)
	include(FetchContent)
    FetchContent_Declare(
        harfbuzz-s
        GIT_REPOSITORY https://github.com/harfbuzz/harfbuzz.git
        GIT_TAG ${HB_VERSION}
    )

    FetchContent_MakeAvailable(harfbuzz-s)
endfunction()