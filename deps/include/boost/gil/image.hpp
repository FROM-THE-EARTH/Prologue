//
// Copyright 2005-2007 Adobe Systems Incorporated
//
// Distributed under the Boost Software License, Version 1.0
// See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt
//
#ifndef BOOST_GIL_IMAGE_HPP
#define BOOST_GIL_IMAGE_HPP

#include <boost/gil/algorithm.hpp>
#include <boost/gil/image_view.hpp>
#include <boost/gil/metafunctions.hpp>

#include <boost/mpl/arithmetic.hpp>
#include <boost/mpl/bool.hpp>
#include <boost/mpl/if.hpp>

#include <cstddef>
#include <memory>

namespace boost { namespace gil {

////////////////////////////////////////////////////////////////////////////////////////
/// \ingroup ImageModel PixelBasedModel
/// \brief container interface over image view. Models ImageConcept, PixelBasedConcept
///
/// A 2D container whose elements are pixels. It is templated over the pixel type, a boolean
/// indicating whether it should be planar, and an optional allocator.
///
/// Note that its element type does not have to be a pixel. \p image can be instantiated with any Regular element,
/// in which case it models the weaker RandomAccess2DImageConcept and does not model PixelBasedConcept
///
/// When recreating an image of the same or smaller size the memory will be reused if possible.
///
////////////////////////////////////////////////////////////////////////////////////////

template< typename Pixel, bool IsPlanar = false, typename Alloc=std::allocator<unsigned char> >
class image {
public:
#if defined(BOOST_NO_CXX11_ALLOCATOR)
    using allocator_type = typename Alloc::template rebind<unsigned char>::other;
#else
    using allocator_type = typename std::allocator_traits<Alloc>::template rebind_alloc<unsigned char>;
#endif
    using view_t = typename view_type_from_pixel<Pixel, IsPlanar>::type;
    using const_view_t = typename view_t::const_t;
    using point_t = typename view_t::point_t;
    using coord_t = typename view_t::coord_t;
    using value_type = typename view_t::value_type;
    using x_coord_t = coord_t;
    using y_coord_t = coord_t;

    const point_t&          dimensions()            const { return _view.dimensions(); }
    x_coord_t               width()                 const { return _view.width(); }
    y_coord_t               height()                const { return _view.height(); }

    explicit image(std::size_t alignment=0,
                   const Alloc alloc_in = Alloc()) :
        _memory(nullptr), _align_in_bytes(alignment), _alloc(alloc_in), _allocated_bytes( 0 ) {}

    // Create with dimensions and optional initial value and alignment
    image(const point_t& dimensions,
          std::size_t alignment=0,
          const Alloc alloc_in = Alloc()) : _memory(nullptr), _align_in_bytes(alignment), _alloc(alloc_in)
                                          , _allocated_bytes( 0 ) {
        allocate_and_default_construct(dimensions);
    }

    image(x_coord_t width, y_coord_t height,
          std::size_t alignment=0,
          const Alloc alloc_in = Alloc()) : _memory(nullptr), _align_in_bytes(alignment), _alloc(alloc_in)
                                          , _allocated_bytes( 0 ) {
        allocate_and_default_construct(point_t(width,height));
    }

    image(const point_t& dimensions,
          const Pixel& p_in,
          std::size_t alignment,
          const Alloc alloc_in = Alloc())  : _memory(nullptr), _align_in_bytes(alignment), _alloc(alloc_in)
                                           , _allocated_bytes( 0 ) {
        allocate_and_fill(dimensions, p_in);
    }
    image(x_coord_t width, y_coord_t height,
          const Pixel& p_in,
          std::size_t alignment = 0,
          const Alloc alloc_in = Alloc())  : _memory(nullptr), _align_in_bytes(alignment), _alloc(alloc_in)
                                           , _allocated_bytes ( 0 ) {
        allocate_and_fill(point_t(width,height),p_in);
    }

    image(const image& img) : _memory(nullptr), _align_in_bytes(img._align_in_bytes), _alloc(img._alloc)
                            , _allocated_bytes( img._allocated_bytes ) {
        allocate_and_copy(img.dimensions(),img._view);
    }

    template <typename P2, bool IP2, typename Alloc2>
    image(const image<P2,IP2,Alloc2>& img) : _memory(nullptr), _align_in_bytes(img._align_in_bytes), _alloc(img._alloc)
                                           , _allocated_bytes( img._allocated_bytes ) {
       allocate_and_copy(img.dimensions(),img._view);
    }

    image& operator=(const image& img) {
        if (dimensions() == img.dimensions())
            copy_pixels(img._view,_view);
        else {
            image tmp(img);
            swap(tmp);
        }
        return *this;
    }

    template <typename Img>
    image& operator=(const Img& img) {
        if (dimensions() == img.dimensions())
            copy_pixels(img._view,_view);
        else {
            image tmp(img);
            swap(tmp);
        }
        return *this;
    }

    ~image() {
        destruct_pixels(_view);
        deallocate();
    }

    Alloc&       allocator() { return _alloc; }
    Alloc const& allocator() const { return _alloc; }

    void swap(image& img) { // required by MutableContainerConcept
        using std::swap;
        swap(_align_in_bytes,  img._align_in_bytes);
        swap(_memory,          img._memory);
        swap(_view,            img._view);
        swap(_alloc,           img._alloc);
        swap(_allocated_bytes, img._allocated_bytes );
    }

    /////////////////////
    // recreate
    /////////////////////

    // without Allocator

    void recreate( const point_t& dims, std::size_t alignment = 0 )
    {
        if( dims == _view.dimensions() && _align_in_bytes == alignment )
        {
            return;
        }

        _align_in_bytes = alignment;

        if( _allocated_bytes >= total_allocated_size_in_bytes( dims ) )
        {
            destruct_pixels( _view );

            create_view( dims
                       , typename mpl::bool_<IsPlanar>()
                       );

            default_construct_pixels( _view );
        }
        else
        {
            image tmp( dims, alignment );
            swap( tmp );
        }
    }

    void recreate( x_coord_t width, y_coord_t height, std::size_t alignment = 0 )
    {
        recreate( point_t( width, height ), alignment );
    }


    void recreate( const point_t& dims, const Pixel& p_in, std::size_t alignment = 0 )
    {
        if( dims == _view.dimensions() && _align_in_bytes == alignment )
        {
            return;
        }

        _align_in_bytes = alignment;

        if(  _allocated_bytes >= total_allocated_size_in_bytes( dims ) )
        {
            destruct_pixels( _view );

            create_view( dims
                       , typename mpl::bool_<IsPlanar>()
                       );

            uninitialized_fill_pixels(_view, p_in);
        }
        else
        {
            image tmp( dims, p_in, alignment );
            swap( tmp );
        }
    }

    void recreate( x_coord_t width, y_coord_t height, const Pixel& p_in, std::size_t alignment = 0 )
    {
        recreate( point_t( width, height ), p_in, alignment );
    }


    // with Allocator
    void recreate(const point_t& dims, std::size_t alignment, const Alloc alloc_in )
    {
        if(  dims            == _view.dimensions()
          && _align_in_bytes == alignment
          && alloc_in        == _alloc
          )
        {
            return;
        }

        _align_in_bytes = alignment;

        if(  _allocated_bytes >= total_allocated_size_in_bytes( dims ) )
        {
            destruct_pixels( _view );

            create_view( dims
                       , typename mpl::bool_<IsPlanar>()
                       );

            default_construct_pixels( _view );
        }
        else
        {
            image tmp( dims, alignment, alloc_in );
            swap( tmp );
        }
    }

    void recreate( x_coord_t width, y_coord_t height, std::size_t alignment, const Alloc alloc_in )
    {
        recreate( point_t( width, height ), alignment, alloc_in );
    }

    void recreate(const point_t& dims, const Pixel& p_in, std::size_t alignment, const Alloc alloc_in )
    {
        if(  dims            == _view.dimensions()
          && _align_in_bytes == alignment
          && alloc_in        == _alloc
          )
        {
            return;
        }

        _align_in_bytes = alignment;

        if(  _allocated_bytes >= total_allocated_size_in_bytes( dims ) )
        {
            destruct_pixels( _view );

            create_view( dims
                       , typename mpl::bool_<IsPlanar>()
                       );

            uninitialized_fill_pixels(_view, p_in);
        }
        else
        {
            image tmp( dims, p_in, alignment, alloc_in );
            swap( tmp );
        }
    }

    void recreate(x_coord_t width, y_coord_t height, const Pixel& p_in, std::size_t alignment, const Alloc alloc_in )
    {
        recreate( point_t( width, height ), p_in,alignment, alloc_in );
    }



    view_t       _view;      // contains pointer to the pixels, the image size and ways to navigate pixels
private:
    unsigned char* _memory;
    std::size_t    _align_in_bytes;
    allocator_type _alloc;

    std::size_t _allocated_bytes;


    void allocate_and_default_construct(const point_t& dimensions) {
        try {
            allocate_(dimensions,mpl::bool_<IsPlanar>());
            default_construct_pixels(_view);
        } catch(...) { deallocate(); throw; }
    }

    void allocate_and_fill(const point_t& dimensions, const Pixel& p_in) {
        try {
            allocate_(dimensions,mpl::bool_<IsPlanar>());
            uninitialized_fill_pixels(_view, p_in);
        } catch(...) { deallocate(); throw; }
    }

    template <typename View>
    void allocate_and_copy(const point_t& dimensions, const View& v) {
        try {
            allocate_(dimensions,mpl::bool_<IsPlanar>());
            uninitialized_copy_pixels(v,_view);
        } catch(...) { deallocate(); throw; }
    }

    void deallocate() {
        if (_memory && _allocated_bytes > 0 )
        {
            _alloc.deallocate(_memory, _allocated_bytes );
        }
    }

    std::size_t is_planar_impl( const std::size_t size_in_units
                              , const std::size_t channels_in_image
                              , mpl::true_
                              ) const
    {
        return size_in_units * channels_in_image;
    }

    std::size_t is_planar_impl( const std::size_t size_in_units
                              , const std::size_t
                              , mpl::false_
                              ) const
    {
        return size_in_units;
    }

    std::size_t total_allocated_size_in_bytes(const point_t& dimensions) const {

        using x_iterator = typename view_t::x_iterator;

        // when value_type is a non-pixel, like int or float, num_channels< ... > doesn't work.
        const std::size_t _channels_in_image = mpl::eval_if< is_pixel< value_type >
                                                           , num_channels< view_t >
                                                           , mpl::int_< 1 >
														   >::type::value;

        std::size_t size_in_units = is_planar_impl( get_row_size_in_memunits( dimensions.x ) * dimensions.y
                                                  , _channels_in_image
                                                  , typename mpl::bool_<IsPlanar>()
                                                  );

        // return the size rounded up to the nearest byte
        return ( size_in_units + byte_to_memunit< x_iterator >::value - 1 )
            / byte_to_memunit<x_iterator>::value
            + ( _align_in_bytes > 0 ? _align_in_bytes - 1 : 0 ); // add extra padding in case we need to align the first image pixel
    }

    std::size_t get_row_size_in_memunits(x_coord_t width) const {   // number of units per row
        std::size_t size_in_memunits = width*memunit_step(typename view_t::x_iterator());
        if (_align_in_bytes>0) {
            std::size_t alignment_in_memunits=_align_in_bytes*byte_to_memunit<typename view_t::x_iterator>::value;
            return align(size_in_memunits, alignment_in_memunits);
        }
        return size_in_memunits;
    }

    void allocate_(const point_t& dimensions, mpl::false_) {  // if it throws and _memory!=0 the client must deallocate _memory

        _allocated_bytes = total_allocated_size_in_bytes(dimensions);
        _memory=_alloc.allocate( _allocated_bytes );

        unsigned char* tmp=(_align_in_bytes>0) ? (unsigned char*)align((std::size_t)_memory,_align_in_bytes) : _memory;
        _view=view_t(dimensions,typename view_t::locator(typename view_t::x_iterator(tmp),get_row_size_in_memunits(dimensions.x)));
    }

    void allocate_(const point_t& dimensions, mpl::true_) {   // if it throws and _memory!=0 the client must deallocate _memory
        std::size_t row_size=get_row_size_in_memunits(dimensions.x);
        std::size_t plane_size=row_size*dimensions.y;

        _allocated_bytes = total_allocated_size_in_bytes( dimensions );

        _memory = _alloc.allocate( _allocated_bytes );

        unsigned char* tmp=(_align_in_bytes>0) ? (unsigned char*)align((std::size_t)_memory,_align_in_bytes) : _memory;
        typename view_t::x_iterator first;
        for (int i=0; i<num_channels<view_t>::value; ++i) {
            dynamic_at_c(first,i) = (typename channel_type<view_t>::type*)tmp;
            memunit_advance(dynamic_at_c(first,i), plane_size*i);
        }
        _view=view_t(dimensions, typename view_t::locator(first, row_size));
    }

    void create_view( const point_t& dims
                    , mpl::true_ // is planar
                    )
    {
        std::size_t row_size=get_row_size_in_memunits(dims.x);
        std::size_t plane_size=row_size*dims.y;

        unsigned char* tmp = ( _align_in_bytes > 0 ) ? (unsigned char*) align( (std::size_t) _memory
                                                                             ,_align_in_bytes
                                                                             )
                                                     : _memory;
        typename view_t::x_iterator first;

        for (int i = 0; i < num_channels< view_t >::value; ++i )
        {
            dynamic_at_c( first, i ) = (typename channel_type<view_t>::type*) tmp;

            memunit_advance( dynamic_at_c(first,i)
                           , plane_size*i
                           );
        }

        _view=view_t( dims
                    , typename view_t::locator( first
                                              , row_size
                                              )
                    );
    }

    void create_view( const point_t& dims
                    , mpl::false_ // is planar
                    )
    {
        unsigned char* tmp = ( _align_in_bytes > 0 ) ? ( unsigned char* ) align( (std::size_t) _memory
                                                                               , _align_in_bytes
                                                                               )
                                                     : _memory;

        _view = view_t( dims
                      , typename view_t::locator( typename view_t::x_iterator( tmp )
                                                , get_row_size_in_memunits( dims.x )
                                                )
                      );
    }
};

template <typename Pixel, bool IsPlanar, typename Alloc>
void swap(image<Pixel, IsPlanar, Alloc>& im1,image<Pixel, IsPlanar, Alloc>& im2) {
    im1.swap(im2);
}

template <typename Pixel1, bool IsPlanar1, typename Alloc1, typename Pixel2, bool IsPlanar2, typename Alloc2>
bool operator==(const image<Pixel1,IsPlanar1,Alloc1>& im1,const image<Pixel2,IsPlanar2,Alloc2>& im2) {
    if ((void*)(&im1)==(void*)(&im2)) return true;
    if (const_view(im1).dimensions()!=const_view(im2).dimensions()) return false;
    return equal_pixels(const_view(im1),const_view(im2));
}
template <typename Pixel1, bool IsPlanar1, typename Alloc1, typename Pixel2, bool IsPlanar2, typename Alloc2>
bool operator!=(const image<Pixel1,IsPlanar1,Alloc1>& im1,const image<Pixel2,IsPlanar2,Alloc2>& im2) {return !(im1==im2);}

///@{
/// \name view, const_view
/// \brief Get an image view from an image

/// \ingroup ImageModel

/// \brief Returns the non-constant-pixel view of an image
template <typename Pixel, bool IsPlanar, typename Alloc> inline
const typename image<Pixel,IsPlanar,Alloc>::view_t& view(image<Pixel,IsPlanar,Alloc>& img) { return img._view; }

/// \brief Returns the constant-pixel view of an image
template <typename Pixel, bool IsPlanar, typename Alloc> inline
const typename image<Pixel,IsPlanar,Alloc>::const_view_t const_view(const image<Pixel,IsPlanar,Alloc>& img) {
    return static_cast<const typename image<Pixel,IsPlanar,Alloc>::const_view_t>(img._view);
}
///@}

/////////////////////////////
//  PixelBasedConcept
/////////////////////////////

template <typename Pixel, bool IsPlanar, typename Alloc>
struct channel_type<image<Pixel,IsPlanar,Alloc> > : public channel_type<Pixel> {};

template <typename Pixel, bool IsPlanar, typename Alloc>
struct color_space_type<image<Pixel,IsPlanar,Alloc> >  : public color_space_type<Pixel> {};

template <typename Pixel, bool IsPlanar, typename Alloc>
struct channel_mapping_type<image<Pixel,IsPlanar,Alloc> > : public channel_mapping_type<Pixel> {};

template <typename Pixel, bool IsPlanar, typename Alloc>
struct is_planar<image<Pixel,IsPlanar,Alloc> > : public mpl::bool_<IsPlanar> {};

}}  // namespace boost::gil

#endif
