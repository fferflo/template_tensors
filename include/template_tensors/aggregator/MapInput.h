#pragma once

namespace aggregator {

namespace detail {

template <typename TFunctor, typename TAggregator>
class map_input : public aggregator::IsAggregator
{
private:
  ::tuple::CompressedPair<TFunctor, TAggregator> m_functor_and_aggregator;

public:
  template <typename TFunctor2, typename TAggregator2>
  __host__ __device__
  map_input(TFunctor2&& functor, TAggregator2&& aggregator)
    : m_functor_and_aggregator(util::forward<TFunctor2>(functor), util::forward<TAggregator2>(aggregator))
  {
  }

  __host__ __device__
  map_input()
  {
  }

  template <typename... TInput>
  __host__ __device__
  void operator()(TInput&&... input)
  {
    m_functor_and_aggregator.second()(m_functor_and_aggregator.first()(util::forward<TInput>(input)...));
  }

  __host__ __device__
  auto get() const
  RETURN_AUTO(m_functor_and_aggregator.second().get())
};

} // end of ns detail

template <typename TFunctor, typename TAggregator>
__host__ __device__
auto map_input(TFunctor&& functor, TAggregator&& aggregator)
RETURN_AUTO(detail::map_input<util::store_member_t<TFunctor&&>, util::store_member_t<TAggregator&&>>(util::forward<TFunctor>(functor), util::forward<TAggregator>(aggregator)))


} // end of ns aggregator