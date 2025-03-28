//------------------------------- -*- C++ -*- -------------------------------//
// Copyright Celeritas contributors: see top-level COPYRIGHT file for details
// SPDX-License-Identifier: (Apache-2.0 OR MIT)
//---------------------------------------------------------------------------//
//! \file geocel/rasterize/ImageWriter.hh
//---------------------------------------------------------------------------//
#pragma once

#include <memory>
#include <string>
#include <vector>

#include "corecel/Config.hh"

#include "corecel/cont/Array.hh"
#include "corecel/cont/Span.hh"
#include "geocel/Types.hh"

#include "Color.hh"

namespace celeritas
{
//---------------------------------------------------------------------------//
/*!
 * Write a 2D array of colors as a PNG file.
 *
 * Each row is written progressively. All rows must be written. Currently alpha
 * values are ignored due to my poor understanding of libpng.
 */
class ImageWriter
{
  public:
    //!@{
    //! \name Type aliases
    using SpanColor = Span<Color>;
    //!@}

  public:
    // Construct with a filename and dimensions
    ImageWriter(std::string const& filename, Size2 height_width);

    // Close on destruction
    inline ~ImageWriter();

    // Write a row
    void operator()(Span<Color const>);

    //! Close the file early so that exceptions can be caught
    void close() { this->close_impl(/* validate = */ true); }

    //! Whether the output is available for writing
    explicit operator bool() const { return static_cast<bool>(impl_); }

  private:
    struct Impl;
    struct ImplDeleter
    {
        void operator()(Impl*);
    };

    std::unique_ptr<Impl, ImplDeleter> impl_;
    Size2 size_;
    size_type rows_written_{0};
    std::vector<char> row_buffer_;

    void close_impl(bool validate);
};

//---------------------------------------------------------------------------//
// INLINE DEFINITIONS
//---------------------------------------------------------------------------//
/*!
 * Close on destruction.
 */
ImageWriter::~ImageWriter()
{
    if (*this)
    {
        this->close_impl(/* validate = */ false);
    }
}

//---------------------------------------------------------------------------//
#if !CELERITAS_USE_PNG
inline ImageWriter::ImageWriter(std::string const&, Size2)
{
    CELER_DISCARD(size_);
    CELER_DISCARD(rows_written_);
    CELER_DISCARD(row_buffer_);
    CELER_NOT_CONFIGURED("PNG");
}
inline void ImageWriter::operator()(Span<Color const>)
{
    CELER_ASSERT_UNREACHABLE();
}
inline void ImageWriter::close_impl(bool) {}
inline void ImageWriter::ImplDeleter::operator()(Impl*) {}
#endif

//---------------------------------------------------------------------------//
}  // namespace celeritas
