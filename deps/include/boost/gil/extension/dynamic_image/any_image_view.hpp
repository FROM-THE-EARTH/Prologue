//
// Copyright 2005-2007 Adobe Systems Incorporated
//
// Distributed under the Boost Software License, Version 1.0
// See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt
//
#ifndef BOOST_GIL_EXTENSION_DYNAMIC_IMAGE_ANY_IMAGE_VIEW_HPP
#define BOOST_GIL_EXTENSION_DYNAMIC_IMAGE_ANY_IMAGE_VIEW_HPP

#include <boost/gil/dynamic_step.hpp>
#include <boost/gil/image.hpp>
#include <boost/gil/image_view.hpp>
#include <boost/gil/point.hpp>

#include <boost/variant.hpp>

namespace boost { namespace gil {

namespace detail {
    template <typename View> struct get_const_t { using type = typename View::const_t; };
    template <typename Views> struct views_get_const_t : public mpl::transform<Views, get_const_t<mpl::_1> > {};
}

template <typename View> struct dynamic_xy_step_transposed_type;

namespace detail {

     // works for both image_view and image
    struct any_type_get_num_channels
    {
        using result_type = int;
        template <typename T>
        result_type operator()(const T&) const { return num_channels<T>::value; }
    };

    // works for both image_view and image
    struct any_type_get_dimensions
    {
        using result_type = point<std::ptrdiff_t>;
        template <typename T>
        result_type operator()(const T& v) const { return v.dimensions(); }
    };
}

////////////////////////////////////////////////////////////////////////////////////////
/// CLASS any_image_view
///
/// \ingroup ImageViewModel
/// \brief Represents a run-time specified image view. Models HasDynamicXStepTypeConcept, HasDynamicYStepTypeConcept, Note that this class does NOT model ImageViewConcept
///
/// Represents a view whose type (color space, layout, planar/interleaved organization, etc) can be specified at run time.
/// It is the runtime equivalent of \p image_view.
/// Some of the requirements of ImageViewConcept, such as the \p value_type alias cannot be fulfilled, since the language does not allow runtime type specification.
/// Other requirements, such as access to the pixels, would be inefficient to provide. Thus \p any_image_view does not fully model ImageViewConcept.
/// However, many algorithms provide overloads taking runtime specified views and thus in many cases \p any_image_view can be used in places taking a view.
///
/// To perform an algorithm on any_image_view, put the algorithm in a function object and invoke it by calling \p apply_operation(runtime_view, algorithm_fn);
////////////////////////////////////////////////////////////////////////////////////////
template <typename ImageViewTypes>
class any_image_view : public make_variant_over<ImageViewTypes>::type {
    using parent_t = typename make_variant_over<ImageViewTypes>::type;
public:
    using const_t = any_image_view<typename detail::views_get_const_t<ImageViewTypes>::type>;
    using x_coord_t = std::ptrdiff_t;
    using y_coord_t = std::ptrdiff_t;
    using point_t = point<std::ptrdiff_t>;

    any_image_view()                                                          : parent_t() {}
    template <typename T> explicit any_image_view(const T& obj)               : parent_t(obj) {}
    any_image_view(const any_image_view& v)                                   : parent_t((const parent_t&)v)    {}
    template <typename Types> any_image_view(const any_image_view<Types>& v)  : parent_t((const typename make_variant_over<Types>::type&)v)    {}

    template <typename T> any_image_view&     operator=(const T& obj)                   { parent_t::operator=(obj); return *this; }
    any_image_view&                           operator=(const any_image_view& v)        { parent_t::operator=((const parent_t&)v); return *this;}
    template <typename Types> any_image_view& operator=(const any_image_view<Types>& v) { parent_t::operator=((const typename make_variant_over<Types>::type&)v); return *this;}

    std::size_t num_channels()  const { return apply_operation(*this, detail::any_type_get_num_channels()); }
    point_t     dimensions()    const { return apply_operation(*this, detail::any_type_get_dimensions()); }
    x_coord_t   width()         const { return dimensions().x; }
    y_coord_t   height()        const { return dimensions().y; }
};

/////////////////////////////
//  HasDynamicXStepTypeConcept
/////////////////////////////

template <typename IVTypes>
struct dynamic_x_step_type<any_image_view<IVTypes>>
{
    using type = any_image_view<typename mpl::transform<IVTypes, dynamic_x_step_type<mpl::_1>>::type>;
};

/////////////////////////////
//  HasDynamicYStepTypeConcept
/////////////////////////////

template <typename IVTypes>
struct dynamic_y_step_type<any_image_view<IVTypes>>
{
    using type = any_image_view<typename mpl::transform<IVTypes, dynamic_y_step_type<mpl::_1>>::type>;
};

template <typename IVTypes>
struct dynamic_xy_step_type<any_image_view<IVTypes>>
{
    using type = any_image_view<typename mpl::transform<IVTypes, dynamic_xy_step_type<mpl::_1>>::type>;
};

template <typename IVTypes>
struct dynamic_xy_step_transposed_type<any_image_view<IVTypes>>
{
    using type = any_image_view<typename mpl::transform<IVTypes, dynamic_xy_step_transposed_type<mpl::_1>>::type>;
};

}}  // namespace boost::gil

#endif
