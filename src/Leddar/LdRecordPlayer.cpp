/// *****************************************************************************
/// Module..: Leddar
///
/// \file    LdRecordPlayer.cpp
///
/// \brief   Functions implementation of class LdRecordPlayer
///
/// \author  David Levy
///
/// \since   May 2018
///
/// Copyright (c) 2018 LeddarTech Inc. All rights reserved.
/// *****************************************************************************

#include "LdRecordPlayer.h"

#include "LdSensor.h"
#include "LdRecordReader.h"
#include "LdLjrRecordReader.h"

#include "LtFileUtils.h"
#include "LtStringUtils.h"

/// *****************************************************************************
/// Function: LdRecordPlayer::LdRecordPlayer
///
/// \brief   Public constructor - Used directly, child classes should use the empty constructor
///
/// \param  aFile   Path to the file to read
///
/// \exception  logic_error (on unrecognised record)
///
/// \author  David Levy
///
/// \since   May 2018
/// *****************************************************************************
LeddarRecord::LdRecordPlayer::LdRecordPlayer( const std::string &aFile ) :
    mSensor( nullptr ),
    mReader( nullptr )
{
    mReader = FileToReader( aFile );

    if( !mReader )
    {
        throw std::logic_error( "Could not create record player from that file extension." );
    }

    mSensor = mReader->CreateSensor();
    mEchoes = mSensor->GetResultEchoes();
    mStates = mSensor->GetResultStates();
    mProperties = mSensor->GetProperties();
}

/// *****************************************************************************
/// Function: LdRecordPlayer::LdRecordPlayer
///
/// \brief   Empty protected constructor - Everything is delegated to child constructor
///
/// \author  David Levy
///
/// \since   May 2018
/// *****************************************************************************
LeddarRecord::LdRecordPlayer::LdRecordPlayer() :
    mSensor( nullptr ),
    mReader( nullptr ),
    mEchoes( nullptr ),
    mStates( nullptr ),
    mProperties( nullptr )
{
}

/// *****************************************************************************
/// Function: LdRecordPlayer::~LdRecordPlayer
///
/// \brief   Destructor
///
/// \author  David Levy
///
/// \since   May 2018
/// *****************************************************************************
LeddarRecord::LdRecordPlayer::~LdRecordPlayer()
{
    if( mReader != nullptr )
    {
        delete mReader;
        mReader = nullptr;
    }
}

/// *****************************************************************************
/// Function: LdPrvLtlRecordReader::ReadNext
///
/// \brief   Read next element in record
///
/// \author  David Levy
///
/// \since   May 2018
/// *****************************************************************************
void
LeddarRecord::LdRecordPlayer::ReadNext()
{
    mReader->ReadNext();
}

/// *****************************************************************************
/// Function: LdPrvLtlRecordReader::ReadPrevious
///
/// \brief   Read Previous element in record
///
/// \author  David Levy
///
/// \since   May 2018
/// *****************************************************************************
void
LeddarRecord::LdRecordPlayer::ReadPrevious()
{
    mReader->ReadPrevious();
}

/// *****************************************************************************
/// Function: LdPrvLtlRecordReader::MoveTo
///
/// \brief   Move to a specific frame in record
///
/// \param  aFrame Number of the frame to move to
///
/// \author  David Levy
///
/// \since   May 2018
/// *****************************************************************************
void
LeddarRecord::LdRecordPlayer::MoveTo( uint32_t aFrame )
{
    mReader->MoveTo( aFrame );
}

/// *****************************************************************************
/// Function: LdPrvLtlRecordReader::GetRecordSize
///
/// \brief   Returns the number of frames in the record
///
/// \author  David Levy
///
/// \since   May 2018
/// *****************************************************************************
uint32_t LeddarRecord::LdRecordPlayer::GetRecordSize() const
{
    return mReader->GetRecordSize();
}

/// *****************************************************************************
/// Function: LdRecordPlayer::FileToReader
///
/// \brief   Return a pointer to the reader associated with that file - Release ownership of the pointer
///
/// \param  aFile   Path to the file to read
///
/// \author  David Levy
///
/// \since   May 2018
/// *****************************************************************************
LeddarRecord::LdRecordReader *
LeddarRecord::LdRecordPlayer::FileToReader( const std::string &aFile )
{
    std::string lExtension = LeddarUtils::LtStringUtils::ToLower( LeddarUtils::LtFileUtils::FileExtension( aFile ) );

    if( lExtension == "ljr" ) //leddar json record
    {
        return new LeddarRecord::LdLjrRecordReader( aFile );
    }

    return nullptr;
}
