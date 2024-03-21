/*
 * Copyright (C) 2016 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "include/flac_parser.h"

#include <android/log.h>
#include <jni.h>

#include <cassert>
#include <cstdlib>
#include <cstring>

#define LOG_TAG "FLACParser"
#define ALOGE(...) \
  ((void)__android_log_print(ANDROID_LOG_ERROR, LOG_TAG, __VA_ARGS__))
#define ALOGV(...) \
  ((void)__android_log_print(ANDROID_LOG_VERBOSE, LOG_TAG, __VA_ARGS__))

#define LOG_ALWAYS_FATAL(...) \
  (__android_log_assert(NULL, LOG_TAG, ##__VA_ARGS__))

#define LITERAL_TO_STRING_INTERNAL(x) #x
#define LITERAL_TO_STRING(x) LITERAL_TO_STRING_INTERNAL(x)

#define TRESPASS()          \
  LOG_ALWAYS_FATAL(__FILE__ \
                   ":" LITERAL_TO_STRING(__LINE__) " Should not be here.");
#define CHECK(x) \
  if (!(x)) ALOGE("Check failed: %s ", #x)

const int endian = 1;
#define isBigEndian() (*(reinterpret_cast<const char *>(&endian)) == 0)

// inside FLAC__stream_decoder_process_until_end_of_metadata
// and FLAC__stream_decoder_process_single.
// We immediately then call our corresponding C++ instance methods
// with the same parameter list, but discard redundant information.

FLAC__StreamDecoderReadStatus FLACParser::read_callback(
        const FLAC__StreamDecoder * /* decoder */, FLAC__byte buffer[],
        size_t *bytes, void *client_data) {
  return reinterpret_cast<FLACParser *>(client_data)
          ->readCallback(buffer, bytes);
}

FLAC__StreamDecoderSeekStatus FLACParser::seek_callback(
        const FLAC__StreamDecoder * /* decoder */,
        FLAC__uint64 absolute_byte_offset, void *client_data) {
  return reinterpret_cast<FLACParser *>(client_data)
          ->seekCallback(absolute_byte_offset);
}

FLAC__StreamDecoderTellStatus FLACParser::tell_callback(
        const FLAC__StreamDecoder * /* decoder */,
        FLAC__uint64 *absolute_byte_offset, void *client_data) {
  return reinterpret_cast<FLACParser *>(client_data)
          ->tellCallback(absolute_byte_offset);
}

FLAC__StreamDecoderLengthStatus FLACParser::length_callback(
        const FLAC__StreamDecoder * /* decoder */, FLAC__uint64 *stream_length,
        void *client_data) {
  return reinterpret_cast<FLACParser *>(client_data)
          ->lengthCallback(stream_length);
}

FLAC__bool FLACParser::eof_callback(const FLAC__StreamDecoder * /* decoder */,
                                    void *client_data) {
  return reinterpret_cast<FLACParser *>(client_data)->eofCallback();
}

FLAC__StreamDecoderWriteStatus FLACParser::write_callback(
        const FLAC__StreamDecoder * /* decoder */, const FLAC__Frame *frame,
        const FLAC__int32 *const buffer[], void *client_data) {
  return reinterpret_cast<FLACParser *>(client_data)
          ->writeCallback(frame, buffer);
}

void FLACParser::metadata_callback(const FLAC__StreamDecoder * /* decoder */,
                                   const FLAC__StreamMetadata *metadata,
                                   void *client_data) {
  reinterpret_cast<FLACParser *>(client_data)->metadataCallback(metadata);
}

void FLACParser::error_callback(const FLAC__StreamDecoder * /* decoder */,
                                FLAC__StreamDecoderErrorStatus status,
                                void *client_data) {
  reinterpret_cast<FLACParser *>(client_data)->errorCallback(status);
}


FLAC__StreamDecoderReadStatus FLACParser::readCallback(FLAC__byte buffer[],
                                                       size_t *bytes) {
  size_t requested = *bytes;
  ssize_t actual = mDataSource->readAt(mCurrentPos, buffer, requested);
  if (0 > actual) {
    *bytes = 0;
    return FLAC__STREAM_DECODER_READ_STATUS_ABORT;
  } else if (0 == actual) {
    *bytes = 0;
    mEOF = true;
    return FLAC__STREAM_DECODER_READ_STATUS_END_OF_STREAM;
  } else {
    assert(actual <= requested);
    *bytes = actual;
    mCurrentPos += actual;
    return FLAC__STREAM_DECODER_READ_STATUS_CONTINUE;
  }
}

FLAC__StreamDecoderSeekStatus FLACParser::seekCallback(
        FLAC__uint64 absolute_byte_offset) {
  mCurrentPos = absolute_byte_offset;
  mEOF = false;
  return FLAC__STREAM_DECODER_SEEK_STATUS_OK;
}

FLAC__StreamDecoderTellStatus FLACParser::tellCallback(
        FLAC__uint64 *absolute_byte_offset) {
  *absolute_byte_offset = mCurrentPos;
  return FLAC__STREAM_DECODER_TELL_STATUS_OK;
}

FLAC__StreamDecoderLengthStatus FLACParser::lengthCallback(
        FLAC__uint64 *stream_length) {
  return FLAC__STREAM_DECODER_LENGTH_STATUS_UNSUPPORTED;
}

FLAC__bool FLACParser::eofCallback() { return mEOF; }

FLAC__StreamDecoderWriteStatus FLACParser::writeCallback(
        const FLAC__Frame *frame, const FLAC__int32 *const buffer[]) {
  if (mWriteRequested) {
    mWriteRequested = false;

    mWriteHeader = frame->header;
    mWriteBuffer = buffer;
    mWriteCompleted = true;
    return FLAC__STREAM_DECODER_WRITE_STATUS_CONTINUE;
  } else {
    ALOGE("FLACParser::writeCallback unexpected");
    return FLAC__STREAM_DECODER_WRITE_STATUS_ABORT;
  }
}

void FLACParser::metadataCallback(const FLAC__StreamMetadata *metadata) {
  switch (metadata->type) {
    case FLAC__METADATA_TYPE_STREAMINFO:
      if (!mStreamInfoValid) {
        mStreamInfo = metadata->data.stream_info;
        mStreamInfoValid = true;
      } else {
        ALOGE("FLACParser::metadataCallback unexpected STREAMINFO");
      }
          break;
    case FLAC__METADATA_TYPE_SEEKTABLE:
      mSeekTable = &metadata->data.seek_table;
          break;
    case FLAC__METADATA_TYPE_VORBIS_COMMENT:
      if (!mVorbisCommentsValid) {
        FLAC__StreamMetadata_VorbisComment vorbisComment =
                metadata->data.vorbis_comment;
        for (FLAC__uint32 i = 0; i < vorbisComment.num_comments; ++i) {
          FLAC__StreamMetadata_VorbisComment_Entry vorbisCommentEntry =
                  vorbisComment.comments[i];
          if (vorbisCommentEntry.entry != NULL) {
            std::string comment(
                    reinterpret_cast<char *>(vorbisCommentEntry.entry),
                    vorbisCommentEntry.length);
            mVorbisComments.push_back(comment);
          }
        }
        mVorbisCommentsValid = true;
      } else {
        ALOGE("FLACParser::metadataCallback unexpected VORBISCOMMENT");
      }
          break;
    case FLAC__METADATA_TYPE_PICTURE: {
      const FLAC__StreamMetadata_Picture *parsedPicture =
              &metadata->data.picture;
      FlacPicture picture;
      picture.mimeType.assign(std::string(parsedPicture->mime_type));
      picture.description.assign(
              std::string((char *)parsedPicture->description));
      picture.data.assign(parsedPicture->data,
                          parsedPicture->data + parsedPicture->data_length);
      picture.width = parsedPicture->width;
      picture.height = parsedPicture->height;
      picture.depth = parsedPicture->depth;
      picture.colors = parsedPicture->colors;
      picture.type = parsedPicture->type;
      mPictures.push_back(picture);
      mPicturesValid = true;
      break;
    }
    default:
      ALOGE("FLACParser::metadataCallback unexpected type %u", metadata->type);
          break;
  }
}

void FLACParser::errorCallback(FLAC__StreamDecoderErrorStatus status) {
  ALOGE("FLACParser::errorCallback status=%d", status);
  mErrorStatus = status;
}

// correct bit-depth (non-zero padded), interleaved.
// These are candidates for optimization if needed.
static void copyToByteArrayBigEndian(int8_t *dst, const int *const *src,
                                     unsigned bytesPerSample, unsigned nSamples,
                                     unsigned nChannels) {
  for (unsigned i = 0; i < nSamples; ++i) {
    for (unsigned c = 0; c < nChannels; ++c) {



      const int8_t *byteSrc =
              reinterpret_cast<const int8_t *>(&src[c][i]) + 4 - bytesPerSample;
      memcpy(dst, byteSrc, bytesPerSample);
      dst = dst + bytesPerSample;
    }
  }
}

static void copyToByteArrayLittleEndian(int8_t *dst, const int *const *src,
                                        unsigned bytesPerSample,
                                        unsigned nSamples, unsigned nChannels) {
  for (unsigned i = 0; i < nSamples; ++i) {
    for (unsigned c = 0; c < nChannels; ++c) {



      memcpy(dst, &(src[c][i]), bytesPerSample);
      dst = dst + bytesPerSample;
    }
  }
}

static void copyTrespass(int8_t * /* dst */, const int *const * /* src */,
                         unsigned /* bytesPerSample */, unsigned /* nSamples */,
                         unsigned /* nChannels */) {
  TRESPASS();
}


FLACParser::FLACParser(DataSource *source)
        : mDataSource(source),
          mCopy(copyTrespass),
          mDecoder(NULL),
          mCurrentPos(0LL),
          mEOF(false),
          mStreamInfoValid(false),
          mSeekTable(NULL),
          firstFrameOffset(0LL),
          mVorbisCommentsValid(false),
          mPicturesValid(false),
          mWriteRequested(false),
          mWriteCompleted(false),
          mWriteBuffer(NULL),
          mErrorStatus((FLAC__StreamDecoderErrorStatus)-1) {
  ALOGV("FLACParser::FLACParser");
  memset(&mStreamInfo, 0, sizeof(mStreamInfo));
  memset(&mWriteHeader, 0, sizeof(mWriteHeader));
}

FLACParser::~FLACParser() {
  ALOGV("FLACParser::~FLACParser");
  if (mDecoder != NULL) {
    FLAC__stream_decoder_delete(mDecoder);
    mDecoder = NULL;
  }
}

bool FLACParser::init() {

  mDecoder = FLAC__stream_decoder_new();
  if (mDecoder == NULL) {



    ALOGE("new failed");
    return false;
  }
  FLAC__stream_decoder_set_md5_checking(mDecoder, false);
  FLAC__stream_decoder_set_metadata_ignore_all(mDecoder);
  FLAC__stream_decoder_set_metadata_respond(mDecoder,
                                            FLAC__METADATA_TYPE_STREAMINFO);
  FLAC__stream_decoder_set_metadata_respond(mDecoder,
                                            FLAC__METADATA_TYPE_SEEKTABLE);
  FLAC__stream_decoder_set_metadata_respond(mDecoder,
                                            FLAC__METADATA_TYPE_VORBIS_COMMENT);
  FLAC__stream_decoder_set_metadata_respond(mDecoder,
                                            FLAC__METADATA_TYPE_PICTURE);
  FLAC__StreamDecoderInitStatus initStatus;
  initStatus = FLAC__stream_decoder_init_stream(
          mDecoder, read_callback, seek_callback, tell_callback, length_callback,
          eof_callback, write_callback, metadata_callback, error_callback,
          reinterpret_cast<void *>(this));
  if (initStatus != FLAC__STREAM_DECODER_INIT_STATUS_OK) {


    ALOGE("init_stream failed %d", initStatus);
    return false;
  }
  return true;
}

bool FLACParser::decodeMetadata() {

  if (!FLAC__stream_decoder_process_until_end_of_metadata(mDecoder)) {
    ALOGE("metadata decoding failed");
    return false;
  }

  FLAC__stream_decoder_get_decode_position(mDecoder, &firstFrameOffset);

  if (mStreamInfoValid) {

    if (getChannels() == 0 || getChannels() > 8) {
      ALOGE("unsupported channel count %u", getChannels());
      return false;
    }

    switch (getBitsPerSample()) {
      case 8:
      case 16:
      case 24:
      case 32:
        break;
      default:
        ALOGE("unsupported bits per sample %u", getBitsPerSample());
            return false;
    }

    if (isBigEndian()) {
      mCopy = copyToByteArrayBigEndian;
    } else {
      mCopy = copyToByteArrayLittleEndian;
    }
  } else {
    ALOGE("missing STREAMINFO");
    return false;
  }
  return true;
}

size_t FLACParser::readBuffer(void *output, size_t output_size) {
  mWriteRequested = true;
  mWriteCompleted = false;

  if (!FLAC__stream_decoder_process_single(mDecoder)) {
    ALOGE("FLACParser::readBuffer process_single failed. Status: %s",
          getDecoderStateString());
    return -1;
  }
  if (!mWriteCompleted) {
    if (FLAC__stream_decoder_get_state(mDecoder) !=
        FLAC__STREAM_DECODER_END_OF_STREAM) {
      ALOGE("FLACParser::readBuffer write did not complete. Status: %s",
            getDecoderStateString());
    }
    return -1;
  }

  unsigned blocksize = mWriteHeader.blocksize;
  if (blocksize == 0 || blocksize > getMaxBlockSize()) {
    ALOGE("FLACParser::readBuffer write invalid blocksize %u", blocksize);
    return -1;
  }
  if (mWriteHeader.sample_rate != getSampleRate() ||
      mWriteHeader.channels != getChannels() ||
      mWriteHeader.bits_per_sample != getBitsPerSample()) {
    ALOGE(
            "FLACParser::readBuffer write changed parameters mid-stream: %d/%d/%d "
            "-> %d/%d/%d",
            getSampleRate(), getChannels(), getBitsPerSample(),
            mWriteHeader.sample_rate, mWriteHeader.channels,
            mWriteHeader.bits_per_sample);
    return -1;
  }

  unsigned bytesPerSample = getBitsPerSample() >> 3;
  size_t bufferSize = blocksize * getChannels() * bytesPerSample;
  if (bufferSize > output_size) {
    ALOGE(
            "FLACParser::readBuffer not enough space in output buffer "
            "%zu < %zu",
            output_size, bufferSize);
    return -1;
  }

  (*mCopy)(reinterpret_cast<int8_t *>(output), mWriteBuffer, bytesPerSample,
           blocksize, getChannels());

  CHECK(mWriteHeader.number_type == FLAC__FRAME_NUMBER_TYPE_SAMPLE_NUMBER);

  return bufferSize;
}

bool FLACParser::getSeekPositions(int64_t timeUs,
                                  std::array<int64_t, 4> &result) {
  if (!mSeekTable) {
    return false;
  }

  unsigned sampleRate = getSampleRate();
  int64_t totalSamples = getTotalSamples();
  int64_t targetSampleNumber = (timeUs * sampleRate) / 1000000LL;
  if (targetSampleNumber >= totalSamples) {
    targetSampleNumber = totalSamples - 1;
  }

  FLAC__StreamMetadata_SeekPoint *points = mSeekTable->points;
  unsigned length = mSeekTable->num_points;

  for (unsigned i = length; i != 0; i--) {
    int64_t sampleNumber = points[i - 1].sample_number;
    if (sampleNumber == -1) {  // placeholder
      continue;
    }
    if (sampleNumber <= targetSampleNumber) {
      result[0] = (sampleNumber * 1000000LL) / sampleRate;
      result[1] = firstFrameOffset + points[i - 1].stream_offset;
      if (sampleNumber == targetSampleNumber || i >= length ||
          points[i].sample_number == -1) {  // placeholder

        result[2] = result[0];
        result[3] = result[1];
      } else {
        result[2] = (points[i].sample_number * 1000000LL) / sampleRate;
        result[3] = firstFrameOffset + points[i].stream_offset;
      }
      return true;
    }
  }
  result[0] = 0;
  result[1] = firstFrameOffset;
  result[2] = 0;
  result[3] = firstFrameOffset;
  return true;
}
