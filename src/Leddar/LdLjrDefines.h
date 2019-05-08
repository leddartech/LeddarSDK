////////////////////////////////////////////////////////////////////////////////////////////////////
/// \file   Leddar/LdLjrDefines.h
///
/// \brief  Declares constants for Leddar Json Record (ljr) format
////////////////////////////////////////////////////////////////////////////////////////////////////

#pragma once

/*
The Leddar Json Record (ljr) format use a full valid json by line. All lines are independants.

First line is the file header:
{
    "header": {
        "prot_version": 1,
        "devicetype": 13,
        "timestamp": 1539197928  /// This is the timestamp in second since UNIX epoch
    }
}

Second line is the list of all the properties with the feature F_SAVE:
{
    "prop": [{
        "id": 34,
        "val": "Sensor AJ04011"
    }, {
        "id": 42,
        "enum": {
            "6": 6,
            "28": 28,
            "53": 53,
            "81": 81,
            "100": 100
        },
        "val": 100
    }, {
        "id": 160,
        "signed": false,
        "limits": [0, 10],
        "val": 5
    }]
}

These first two lines are mandatory.

All the following lines correspond to an event: a new frame or a change in the configuration.
For a new frame:
{
    "frame": {
        "ts": 6346777, //Sensor timestamp in milliseconds
        "echoes": [
            [7, 0.36981201171875, 225.76735305786134, 1], ///Channel index, distance, amplitude, flags
            [6, 0.353057861328125, 330.2265930175781, 1],
            [5, 0.2869873046875, 318.28287506103518, 1],
            [4, 0.3236846923828125, 230.51702308654786, 1],
            [3, 0.847076416015625, 312.74399185180666, 1],
            [2, 4.1929931640625, 487.8481254577637, 9],
            [1, 3.4172210693359377, 327.16299057006838, 1],
            [0, 2.55078125, 228.6460666656494, 1]
        ],
        "states": [{ ///List all properties of the states
            "id": 6659,
            "val": 38.5
        }, {
            "id": 5243136,
            "val": 11.300000190734864
        }, {
            "id": 5243140,
            "val": 1
        }]
    }
}
For a change in the configuration (same syntax as the second line of the file - But with a single property):
{
    "prop": [{
        "id": 160,
        "val": 6
    }]
}


*/

namespace LeddarRecord
{
    const unsigned int LJR_PROT_VERSION = 1;
    const unsigned int LJR_HEADER_LINES = 2;
}