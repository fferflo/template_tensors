namespace template_tensors {

namespace detail {

template <size_t I, size_t N>
struct MortonForLoopToIndexHelper
{
  static const size_t FIELD_BITS = sizeof(size_t) * 8 / N;

  template <typename TDimArgType, typename... TCoordArgTypes>
  __host__ __device__
  static void toIndex(size_t& index, TDimArgType&& dims, TCoordArgTypes&&... coords)
  {
    const size_t field = getNthCoordinate<I>(util::forward<TCoordArgTypes>(coords)...);
    ASSERT(N == 1 || ((((size_t) -1) << (N == 1 ? 0 : FIELD_BITS)) & field) == 0, "Coordinate out of morton range");
    ASSERT(N == 1 || getNthDimension<I>(util::forward<TDimArgType>(dims)) < (1UL << (N == 1 ? 0 : FIELD_BITS)), "Dimension out of morton range");

    for (size_t field_bit = 0; field_bit < FIELD_BITS; field_bit++)
    {
      const size_t index_bit = I + field_bit * N;
      index |= (field & (1UL << field_bit)) << (index_bit - field_bit);
    }

    MortonForLoopToIndexHelper<I + 1, N>::toIndex(index, util::forward<TDimArgType>(dims), util::forward<TCoordArgTypes>(coords)...);
  }
};

template <size_t N>
struct MortonForLoopToIndexHelper<N, N>
{
  template <typename TDimArgType, typename... TCoordArgTypes>
  __host__ __device__
  static void toIndex(size_t& index, TDimArgType&& dims, TCoordArgTypes&&... coords)
  {
  }
};

template <size_t I, size_t N>
struct MortonForLoopFromIndexHelper
{
  static const size_t FIELD_BITS = sizeof(size_t) * 8 / N;

  template <size_t TDims, typename... TDimArgTypes>
  __host__ __device__
  static void fromIndex(VectorXs<TDims>& result, size_t index, TDimArgTypes&&... dims)
  {
    ASSERT(N == 1 || template_tensors::getNthDimension<I>(util::forward<TDimArgTypes>(dims)...) < (1UL << (N == 1 ? 0 : FIELD_BITS)), "Dimension out of morton range");

    if (I < TDims)
    {
      size_t field = 0;
      for (size_t field_bit = 0; field_bit < FIELD_BITS; field_bit++)
      {
        const size_t index_bit = I + field_bit * N;
        field |= (index & (1UL << index_bit)) >> (index_bit - field_bit);
      }
      result(I) = field;
    }

    MortonForLoopFromIndexHelper<I + 1, N>::fromIndex(result, index, util::forward<TDimArgTypes>(dims)...);
  }
};

template <size_t N>
struct MortonForLoopFromIndexHelper<N, N>
{
  template <size_t TDims, typename... TDimArgTypes>
  __host__ __device__
  static void fromIndex(VectorXs<TDims>& result, size_t index, TDimArgTypes&&... dims)
  {
    for (size_t i = N; i < TDims; i++)
    {
      result(i) = 0;
    }
  }
};

} // end of ns detail





template <size_t TRank>
struct MortonForLoop
{
  static const size_t FIELD_BITS = sizeof(size_t) * 8 / TRank;
  static const bool IS_STATIC = false;

  template <typename TDimArgType, typename... TCoordArgTypes, ENABLE_IF(are_dim_args_v<TDimArgType&&>::value)>
  __host__ __device__
  size_t toIndex(TDimArgType&& dims, TCoordArgTypes&&... coords) const volatile
  {
    ASSERT(getNonTrivialDimensionsNum(util::forward<TDimArgType>(dims)) <= TRank, "Dimensions out of morton range");
    ASSERT(coordsAreInRange(util::forward<TDimArgType>(dims), util::forward<TCoordArgTypes>(coords)...), "Coordinates are out of range");
    size_t index = 0;
    detail::MortonForLoopToIndexHelper<0, TRank>::toIndex(index, util::forward<TDimArgType>(dims), util::forward<TCoordArgTypes>(coords)...);
    return index;
  }

  TT_INDEXSTRATEGY_TO_INDEX_2

  template <size_t TDimsArg = DYN, typename... TDimArgTypes, size_t TDims = TDimsArg == DYN ? dimension_num_v<TDimArgTypes&&...>::value : TDimsArg>
  __host__ __device__
  VectorXs<TDims> fromIndex(size_t index, TDimArgTypes&&... dims) const volatile
  {
    ASSERT(getNonTrivialDimensionsNum(util::forward<TDimArgTypes>(dims)...) <= TRank, "Dimensions out of morton range");
    VectorXs<TDims> result;
    detail::MortonForLoopFromIndexHelper<0, TRank>::fromIndex(result, index, util::forward<TDimArgTypes>(dims)...);
    return result;
  }

  TT_INDEXSTRATEGY_FROM_INDEX_2

  template <typename... TDimArgTypes, ENABLE_IF(are_dim_args_v<TDimArgTypes...>::value)>
  __host__ __device__
  size_t getSize(TDimArgTypes&&... dim_args) const volatile
  {
    return toIndex(toDimVector(util::forward<TDimArgTypes>(dim_args)...), toDimVector(util::forward<TDimArgTypes>(dim_args)...) - 1) + 1;
  }
};

template <size_t TRank>
__host__ __device__
bool operator==(const volatile MortonForLoop<TRank>&, const volatile MortonForLoop<TRank>&)
{
  return true;
}

HD_WARNING_DISABLE
template <typename TStreamType, size_t TRank>
__host__ __device__
TStreamType&& operator<<(TStreamType&& stream, const MortonForLoop<TRank>& index_strategy)
{
  stream << "MortonForLoop<" << TRank << ">";
  return util::forward<TStreamType>(stream);
}

#ifdef CEREAL_INCLUDED
template <typename TArchive, size_t TRank>
void save(TArchive& archive, const MortonForLoop<TRank>& m)
{
}

template <typename TArchive, size_t TRank>
void load(TArchive& archive, MortonForLoop<TRank>& m)
{
}
#endif

} // end of ns tensor

template <size_t TRank>
TT_PROCLAIM_TRIVIALLY_RELOCATABLE((template_tensors::MortonForLoop<TRank>));
