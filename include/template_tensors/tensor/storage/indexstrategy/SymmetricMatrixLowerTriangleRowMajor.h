namespace template_tensors {

/*!
 * \brief An indexing strategy for a symmetric matrix where the lower triangular portion is stored in row major order.
 */
struct SymmetricMatrixLowerTriangleRowMajor
{
  static const bool IS_STATIC = true;

  template <typename TDimArgType, typename... TCoordArgTypes, ENABLE_IF(are_dim_args_v<TDimArgType&&>::value)>
  __host__ __device__
  size_t toIndex(TDimArgType&& dims, TCoordArgTypes&&... coords) const
  {
    ASSERT(getNonTrivialDimensionsNum(util::forward<TDimArgType>(dims)) <= 2, "Not a matrix");
    ASSERT(coordsAreInRange(util::forward<TDimArgType>(dims), util::forward<TCoordArgTypes>(coords)...), "Coordinates are out of range");
    const size_t row = getNthCoordinate<0>(util::forward<TCoordArgTypes>(coords)...);
    const size_t col = getNthCoordinate<1>(util::forward<TCoordArgTypes>(coords)...);
    if (row >= col)
    {
      return ((row + 1) * row >> 1) + col;
    }
    else
    {
      return ((col + 1) * col >> 1) + row;
    }
  }

  TT_INDEXSTRATEGY_TO_INDEX_2

  template <typename... TDimArgTypes>
  __host__ __device__
  constexpr size_t getSize(TDimArgTypes&&... dims) const
  {
    // TODO: ASSERT is symmetric and rest of dimension are 1
    const size_t dim0 = template_tensors::getNthDimension<0>(util::forward<TDimArgTypes>(dims)...);
    return (dim0 * dim0 + dim0) >> 1;
  }

  /*
  // TODO: floating sqrt not safe for big integers
  template <size_t TDimsArg = DYN, typename TDimArgType, size_t TDims = TDimsArg == DYN ? dimension_num_v<TDimArgType>::value : TDimsArg>
  __host__ __device__
  VectorXs<TDims> fromIndex(TDimArgType&& dims, size_t index) const // TODO: put index arg first so that multiple dim args can be passed
  {
    VectorXs<TDims> result;
    const size_t row = (-3 + math::sqrt<float>(9 + 8 * index)) / 2;
    const size_t col = index - row * (row + 1) / 2;
    detail::ColMajorFromIndexHelper<1, TDims>::fromIndex(result, util::forward<TDimArgType>(dims), index);
    return result;
  }

  template <typename TVectorType, typename TElementType, size_t TRank>
  __host__ __device__
  VectorXs<TRank> fromIndex(const Vector<TVectorType, TElementType, TRank>& dims, size_t index) const
  {

  }*/
};

/*!
 * \brief An indexing strategy for a symmetric matrix where the upper triangular portion is stored in column major order.
 */
using SymmetricMatrixUpperTriangleColMajor = SymmetricMatrixLowerTriangleRowMajor;

__host__ __device__
inline bool operator==(const SymmetricMatrixLowerTriangleRowMajor&, const SymmetricMatrixLowerTriangleRowMajor&)
{
  return true;
}

HD_WARNING_DISABLE
template <typename TStreamType>
__host__ __device__
TStreamType&& operator<<(TStreamType&& stream, const SymmetricMatrixLowerTriangleRowMajor& index_strategy)
{
  stream << "SymmetricMatrixLowerTriangleRowMajor";
  return util::forward<TStreamType>(stream);
}

#ifdef CEREAL_INCLUDED
template <typename TArchive>
void save(TArchive& archive, const SymmetricMatrixLowerTriangleRowMajor& m)
{
}

template <typename TArchive>
void load(TArchive& archive, SymmetricMatrixLowerTriangleRowMajor& m)
{
}
#endif

} // end of ns tensor

TT_PROCLAIM_TRIVIALLY_RELOCATABLE_NOTEMPLATE((template_tensors::SymmetricMatrixLowerTriangleRowMajor));
