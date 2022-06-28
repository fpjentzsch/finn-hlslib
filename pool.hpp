/******************************************************************************
 *  Copyright (c) 2019, Xilinx, Inc.
 *  All rights reserved.
 *
 *  Redistribution and use in source and binary forms, with or without
 *  modification, are permitted provided that the following conditions are met:
 *
 *  1.  Redistributions of source code must retain the above copyright notice,
 *     this list of conditions and the following disclaimer.
 *
 *  2.  Redistributions in binary form must reproduce the above copyright
 *      notice, this list of conditions and the following disclaimer in the
 *      documentation and/or other materials provided with the distribution.
 *
 *  3.  Neither the name of the copyright holder nor the names of its
 *      contributors may be used to endorse or promote products derived from
 *      this software without specific prior written permission.
 *
 *  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 *  AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
 *  THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 *  PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR
 *  CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 *  EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 *  PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
 *  OR BUSINESS INTERRUPTION). HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 *  WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
 *  OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF
 *  ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 ******************************************************************************/
 
/******************************************************************************
 *
 *  Authors: Giulio Gambardella <giuliog@xilinx.com>
 *           Tobias Alonso <tobiasa@xilinx.com>
 *  \file pool.hpp
 *
 *  This file defines the pool activations
 *
 ******************************************************************************/

#ifndef POOL_HPP
#define POOL_HPP

#include "activations.hpp"

/*!
 * \brief PoolFunction: General contract for pool functions.
 *
 * This class itself has no formal significance for the implementation
 * of the pool function. It provides a guidence for specific pool function to be used in Pool_batch
 * 
 * \tparam TA Datatype of the internal accumulation in the pool function
 * \tparam TO Datatype of the output generated by the pool function
 * \tparam size Additional optional unsigned parameter to be used in pool or activate
 *
 */
template<typename TA, typename TO, unsigned size>
class PoolFunction {
public:
/*!
 * \brief init: initialization function returning the datatype for the accumulators
*/
  TA init(void) const {
#pragma HLS inline
    return  TA(0);
  }

/*!
 * \brief pool: computes the pooling algorithm (e.g., max, avg, sum)
 *
 * \param input Input value to be used in the pool function 
 * \param accu  Value already computed in previous iterations
*/
  TA pool(TA const &input, TA const &accu) const;
/*!
 * \brief activate: compute the output of pooling algorithm (e.g., max, avg, sum)
 *
 * \param accu Value already computed in previous iterations
*/
  TO activate(TA const &accu) const;
};

/*!
 * \brief MaxPoolFunction: Implementing max pool 
 *
 * This class inherits from the generic Poolfunction to implement Max Pool
 * 
 * \tparam T Datatype of the input value and the accu value containing the previously computed max
 * \tparam size Unused 
 *
 */
template<typename T, unsigned size>
class MaxPoolFunction : public PoolFunction<T, T, size> {
public:

T init(void) const {
#pragma HLS inline
    const T T_MIN_VAL = (T(-1)<0)? 1<<(T::width-1) : 0;
    return  T_MIN_VAL;
  }
/*!
 * \brief pool: computes the max value 
 *
 * \param input Input value to be used in the max pool function 
 * \param accu  Max value already computed in previous iterations
*/
  T pool(T const &input, T const &accu) const{
#pragma HLS inline
    return comp::max<T, T, T>()(input,accu);
  }
/*!
 * \brief activate: compute the output of the max pooling algorithm
 *
 * \param accu Max value already computed and returned
*/  
  T activate(T const &accu) const {
#pragma HLS inline
    return  accu;
  }
};

/*!
 * \brief AvgPoolFunction: Implementing avg pool 
 *
 * This class inherits from the generic Poolfunction to implement Average Pool
 * 
 * \tparam TA Datatype of the internal accumulation in the avg pool function
 * \tparam TO Datatype of the output generated by the avg pool function
 * \tparam size Value used as divisor on the accumulator to generate output 
 *
 */
template<typename TA, typename TO, unsigned size>
class AvgPoolFunction : public PoolFunction<TA, TO, size> {
public:
/*!
 * \brief pool: computes the sum 
 *
 * \param input Input value to be used in the avg pool function 
 * \param accu  Accumulation value already computed in previous iterations
*/
  TA pool(TA const &input, TA const &accu) const{
#pragma HLS inline
    return comp::add<TA, TA, TA>()(input,accu);
  }
/*!
 * \brief activate: compute the output of the avg pooling algorithm
 *
 * \param accu Accumulation value already computed in previous iterations 
*/    
  TO activate(TA const &accu) const {
#pragma HLS inline
    return  (accu/size);
  }
};

/*!
 * \brief AccPoolFunction: Implementing accumulation pool 
 *
 * This class inherits from the generic Poolfunction to implement accumulation Pool
 * 
 * \tparam TA Datatype of the internal accumulation in the avg pool function

 * \tparam size Unused 
 *
 */
template<typename TA, unsigned size>
class AccPoolFunction : public PoolFunction<TA, TA, size> {
public:
/*!
 * \brief pool: computes the sum 
 *
 * \param input Input value to be used in the avg pool function 
 * \param accu  Accumulation value already computed in previous iterations
*/
  TA pool(TA const &input, TA const &accu) const{
#pragma HLS inline
    return comp::add<TA, TA, TA>()(input,accu);
  }
/*!
 * \brief activate: compute the output of the max pooling algorithm
 *
 * \param accu Accumulation value already computed in previous iterations 
*/   
  TA activate(TA const &accu) const {
#pragma HLS inline
    return  accu;
  }
};

/*!
 * \brief QuantAvgPoolFunction: Implementing avg pool with shift instead of 
 * division
 *
 * This class inherits from the generic Poolfunction to implement Average Pool  
 * with shift instead of division
 * 
 * \tparam TA Datatype of the internal accumulation in the quant avg pool function
 * \tparam TO Datatype of the output generated by the quant avg pool function
 * \tparam size Number of right shifts applied to generate output 
 *
 */
template<typename TA, typename TO, unsigned size>
class QuantAvgPoolFunction : public PoolFunction<TA, TO, size> {
public:
/*!
 * \brief pool: computes the sum 
 *
 * \param input Input value to be used in the avg pool function 
 * \param accu  Accumulation value already computed in previous iterations
*/
  TA pool(TA const &input, TA const &accu) const{
#pragma HLS inline
    return input + accu;
  }
/*!
 * \brief activate: compute the output of the quant avg pooling algorithm
 *
 * \param accu Accumulation value already computed in previous iterations
*/    
  TO activate(TA const &accu) const {
#pragma HLS inline
    return  TO(accu>>size);
  }
};

#endif
