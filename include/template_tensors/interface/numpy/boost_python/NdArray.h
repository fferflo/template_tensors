#pragma once

#ifdef BOOST_NUMPY_INCLUDED

#include <jtuple/tuple_utility.hpp>
#include <boost/python/list.hpp>
#include <boost/python/numpy.hpp>

namespace template_tensors::python::boost {

#define ThisType FromPythonNumpyWrapper<TElementType, TRank, TNumpyArray>
#define SuperType IndexedPointerTensor< \
                    ThisType, \
                    TElementType, \
                    template_tensors::Stride<TRank>, \
                    mem::HOST, \
                    dyn_dimseq_t<TRank> \
                  >

template <typename TElementType, metal::int_ TRank, typename TNumpyArray = ::boost::python::numpy::ndarray>
class FromPythonNumpyWrapper : public SuperType
{
private:
  TNumpyArray m_numpy;

public:
  static_assert(std::is_arithmetic<TElementType>::value, "Elementtype is not a valid numpy type");

  __host__
  FromPythonNumpyWrapper(TNumpyArray numpy)
    : SuperType(
        Stride<TRank>(VectorXT<size_t, TRank>(template_tensors::ref<template_tensors::ColMajor, mem::HOST, TRank>(numpy.get_strides()) / sizeof(TElementType))),
        VectorXT<size_t, TRank>(template_tensors::ref<template_tensors::ColMajor, mem::HOST, TRank>(numpy.get_shape()))
      )
    , m_numpy(numpy)
  {
  }

  static ::boost::python::numpy::ndarray make(template_tensors::VectorXT<size_t, TRank> dims)
  {
    template_tensors::python::with_gil guard;
    return ::boost::python::numpy::empty(jtuple::tuple_apply(functor::make_tuple(), template_tensors::toTuple(dims)), ::boost::python::numpy::dtype::get_builtin<TElementType>());
  }

  __host__
  FromPythonNumpyWrapper(template_tensors::VectorXT<size_t, TRank> dims)
    : FromPythonNumpyWrapper(make(dims))
  {
  }

  TT_TENSOR_SUBCLASS_ASSIGN(ThisType)

  HD_WARNING_DISABLE
  template <typename TThisType2>
  __host__ __device__
  static auto data2(TThisType2&& self)
  RETURN_AUTO(reinterpret_cast<typename std::remove_reference<util::copy_qualifiers_t<TElementType, TThisType2>>::type*>(self.m_numpy.get_data()))
  FORWARD_ALL_QUALIFIERS(data, data2)

  template <metal::int_ TIndex>
  __host__
  dim_t getDynDim() const
  {
    return TIndex < TRank ? m_numpy.get_shape()[TIndex] : 1;
  }

  __host__
  dim_t getDynDim(size_t index) const
  {
    return index < TRank ? m_numpy.get_shape()[index] : 1;
  }

  __host__
  TNumpyArray& getNumpyArray()
  {
    return m_numpy;
  }

  __host__
  const TNumpyArray& getNumpyArray() const
  {
    return m_numpy;
  }
};
#undef SuperType
#undef ThisType


template <typename TElementType, metal::int_ TRank>
__host__
FromPythonNumpyWrapper<TElementType, TRank> fromNumpy(::boost::python::numpy::ndarray arr)
{
  {
    template_tensors::python::with_gil guard;
    if (arr.get_nd() != TRank)
    {
      throw template_tensors::python::InvalidNumpyShapeException(arr.get_nd(), TRank);
    }
    if (arr.get_dtype() != ::boost::python::numpy::dtype::get_builtin<TElementType>())
    {
      throw template_tensors::python::InvalidNumpyElementTypeException(arr.get_dtype().get_itemsize(), sizeof(TElementType));
    }
  }

  return FromPythonNumpyWrapper<TElementType, TRank>(arr);
}

template <typename TElementType, metal::int_ TRank>
__host__
FromPythonNumpyWrapper<TElementType, TRank> fromNumpy(::boost::python::numpy::ndarray arr, template_tensors::VectorXT<size_t, TRank> dims)
{
  FromPythonNumpyWrapper<TElementType, TRank> result = fromNumpy<TElementType, TRank>(arr);
  if (!template_tensors::eq(result.template dims<TRank>(), dims))
  {
    throw template_tensors::python::InvalidNumpyShapeException(result.template dims<TRank>(), dims);
  }
  return FromPythonNumpyWrapper<TElementType, TRank>(arr);
}

template <metal::int_ TRank2 = DYN, typename TTensorType, metal::int_ TRank = TRank2 == DYN ? non_trivial_dimensions_num_v<TTensorType>::value : TRank2>
__host__
::boost::python::numpy::ndarray toNumpy(TTensorType&& tensor)
{
  FromPythonNumpyWrapper<decay_elementtype_t<TTensorType>, TRank> result_as_tensor(tensor.template dims<TRank>());
  result_as_tensor = std::forward<TTensorType>(tensor);
  return std::move(result_as_tensor.getNumpyArray());
}

} // end of ns template_tensors::python::boost

#endif