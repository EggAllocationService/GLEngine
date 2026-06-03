//
// Created by Kyle Smith on 2026-06-03.
//
#pragma once


namespace glengine::pak {
    /* General pak file format
     * PakHeader
     * array of entries
     * each entry is laid out as:
     * - unsigned short nameLength
     * - char name[nameLength]
     * - unsigned int length;
     * - char data[length];
     */
    struct PakHeader {
        char magic[4];
        int version;
        int entryCount;
    };
}