#ifndef LP_MP_MULTICUT_FACTORS_MESSAGES_HXX
#define LP_MP_MULTICUT_FACTORS_MESSAGES_HXX

#include "LP_MP.h"
#include "factors/labeling_list_factor.hxx"

namespace LP_MP {

using multicut_edge_labelings = labelings< labeling<1> >;
using multicut_edge_factor = labeling_factor< multicut_edge_labelings, true >;

using multicut_triplet_labelings = labelings<
   labeling<0,1,1>,
   labeling<1,0,1>,
   labeling<1,1,0>,
   labeling<1,1,1>
      >;
using multicut_triplet_factor = labeling_factor< multicut_triplet_labelings, true >;

using multicut_edge_triplet_message_0 = labeling_message< multicut_edge_labelings, multicut_triplet_labelings, 0 >;
using multicut_edge_triplet_message_1 = labeling_message< multicut_edge_labelings, multicut_triplet_labelings, 1 >;
using multicut_edge_triplet_message_2 = labeling_message< multicut_edge_labelings, multicut_triplet_labelings, 2 >;

/*

   ______i1_____
  /      |      \
 /       |       \
/   _____i3____   \
|  /           \  |
| /             \ |
i0_______________i2

   */
using multicut_odd_3_wheel_labelings = labelings<
   // two components
   // one node separated from the rest
   labeling<1,1,0,1,0,0>, // 1000
   labeling<1,0,1,0,1,0>, // 0100
   labeling<0,1,1,0,0,1>, // 0010
   labeling<0,0,0,1,1,1>, // 0001

   // two components of two nodes each
   labeling<0,1,1,1,1,0>, // 1100
   labeling<1,0,1,1,0,1>, // 0101
   labeling<1,1,0,0,1,1>, // 1001

   // three components
   labeling<0,1,1,1,1,1>,
   labeling<1,0,1,1,1,1>,
   labeling<1,1,0,1,1,1>,
   labeling<1,1,1,0,1,1>,
   labeling<1,1,1,1,0,1>,
   labeling<1,1,1,1,1,0>,

   // four components
   labeling<1,1,1,1,1,1>
   >;
using multicut_odd_3_wheel_factor = labeling_factor< multicut_odd_3_wheel_labelings, true >;

using multicut_triplet_odd_3_wheel_message_012 = labeling_message< multicut_triplet_labelings, multicut_odd_3_wheel_labelings, 0,1,2>;
using multicut_triplet_odd_3_wheel_message_013 = labeling_message< multicut_triplet_labelings, multicut_odd_3_wheel_labelings, 0,3,4>;
using multicut_triplet_odd_3_wheel_message_023 = labeling_message< multicut_triplet_labelings, multicut_odd_3_wheel_labelings, 1,3,5>;
using multicut_triplet_odd_3_wheel_message_123 = labeling_message< multicut_triplet_labelings, multicut_odd_3_wheel_labelings, 2,4,5>;

/*
                       i1



               
                    i3_
                       \__
                          i4




   i0                                       i2
   */
// is also the full graph on 5 nodes
using multicut_odd_bicycle_3_wheel_labelings = labelings<
// two components
// one node separated from the rest
labeling<1,1,0,1,1,0,0,0,0,0>, // 10000
labeling<1,0,1,0,0,1,1,0,0,0>, // 01000
labeling<0,1,1,0,0,0,0,1,1,0>, // 00100
labeling<0,0,0,1,0,1,0,1,0,1>, // 00010
labeling<0,0,0,0,1,0,1,0,1,1>, // 00001
// two nodes, three nodes
labeling<0,1,1,1,1,1,1,0,0,0>, // 11000
labeling<1,0,1,1,1,0,0,1,1,0>, // 10100
labeling<1,1,0,0,1,1,0,1,0,1>, // 10010
labeling<1,1,0,1,0,0,1,0,1,1>, // 10001
labeling<1,1,0,0,0,1,1,1,1,0>, // 01100
labeling<1,0,1,1,0,0,1,1,0,1>, // 01010
labeling<1,0,1,0,1,1,0,0,1,1>, // 01001
labeling<0,1,1,0,1,1,0,0,1,1>, // 00101
labeling<0,0,0,1,1,1,1,1,1,0>, // 00011

// three conponents
// two singleton components
labeling<1,1,1,1,1,1,1,0,0,0>, // 01222
labeling<1,1,1,1,1,0,0,1,1,0>, // 02122
labeling<1,1,0,1,1,1,0,1,0,1>, // 02212
labeling<1,1,0,1,1,0,1,0,1,1>, // 02221
labeling<1,1,1,0,0,1,1,1,1,0>, // 20122
labeling<1,0,1,1,0,1,1,1,0,1>, // 20212
labeling<1,0,1,0,1,1,1,0,1,1>, // 20221
labeling<0,1,1,1,0,1,0,1,1,1>, // 22012
labeling<0,1,1,0,1,0,1,1,1,1>, // 22021
labeling<0,0,0,1,1,1,1,1,1,1>, // 22201
// one singleton component, two two-element components
labeling<1,1,0,1,1,1,1,1,1,0>, // 01122
labeling<1,1,1,1,1,0,1,1,0,1>, // 01212
labeling<1,1,1,1,1,1,0,1,0,1>, // 01221
labeling<1,0,1,1,1,1,1,1,1,0>, // 10122
labeling<1,1,1,0,1,1,1,1,0,1>, // 10212
labeling<1,1,1,1,0,1,1,1,0,1>, // 10221
labeling<0,1,1,1,1,1,1,1,1,0>, // 11022
labeling<1,1,1,0,1,1,0,1,1,1>, // 12012
labeling<1,1,1,1,0,0,1,1,1,0>, // 12021
labeling<0,1,1,1,1,1,1,1,0,1>, // 11202
labeling<1,0,1,1,1,1,0,1,1,1>, // 12102
labeling<1,1,0,1,0,1,1,1,1,1>, // 12201
labeling<0,1,1,1,1,1,1,0,1,1>, // 11220
labeling<1,0,1,1,1,0,1,1,1,1>, // 12120
labeling<1,1,0,0,1,1,1,1,1,1>, // 12210

// four components
labeling<0,1,1,1,1,1,1,1,1,1>,
labeling<1,0,1,1,1,1,1,1,1,1>,
labeling<1,1,0,1,1,1,1,1,1,1>,
labeling<1,1,1,0,1,1,1,1,1,1>,
labeling<1,1,1,1,0,1,1,1,1,1>,
labeling<1,1,1,1,1,0,1,1,1,1>,
labeling<1,1,1,1,1,1,0,1,1,1>,
labeling<1,1,1,1,1,1,1,0,1,1>,
labeling<1,1,1,1,1,1,1,1,0,1>,
labeling<1,1,1,1,1,1,1,1,1,0>,
// five components
labeling<1,1,1,1,1,1,1,1,1,1>
   >;
using multicut_odd_bicycle_3_wheel_factor = labeling_factor< multicut_odd_bicycle_3_wheel_labelings, true >;

using multicut_odd_3_wheel_odd_bicycle_message0123 = labeling_message< multicut_odd_3_wheel_factor, multicut_odd_bicycle_3_wheel_factor, 0,1,3,2,5,7 >;
using multicut_odd_3_wheel_odd_bicycle_message0124 = labeling_message< multicut_odd_3_wheel_factor, multicut_odd_bicycle_3_wheel_factor, 0,1,4,2,6,8 >;
using multicut_odd_3_wheel_odd_bicycle_message0134 = labeling_message< multicut_odd_3_wheel_factor, multicut_odd_bicycle_3_wheel_factor, 0,3,4,5,6,9 >;
using multicut_odd_3_wheel_odd_bicycle_message0234 = labeling_message< multicut_odd_3_wheel_factor, multicut_odd_bicycle_3_wheel_factor, 1,3,4,7,8,9 >;
using multicut_odd_3_wheel_odd_bicycle_message1234 = labeling_message< multicut_odd_3_wheel_factor, multicut_odd_bicycle_3_wheel_factor, 2,5,6,7,8,9 >;

/*
using multicut_triplet_plus_spoke_labelings = labelings<
   labeling<0,1,1,0>,
   labeling<1,0,1,0>,
   labeling<1,1,0,0>,
   labeling<1,1,1,0>,
   labeling<0,1,1,1>,
   labeling<1,0,1,1>,
   labeling<1,1,0,1>,
   labeling<1,1,1,1>,
   labeling<0,0,0,1>
      >;
using multicut_triplet_plus_spoke_factor = labeling_factor< multicut_triplet_plus_spoke_labelings, true >; 

using multicut_triplet_plus_spoke_cover_message = labeling_message< multicut_triplet_labelings, multicut_triplet_plus_spoke_labelings, 0,1,2 >;
using multicut_triplet_plus_spoke_message = labeling_message< multicut_triplet_labelings, multicut_triplet_plus_spoke_labelings, 0,1,2 >;
*/

} // end namespace LP_MP 

#endif // LP_MP_MULTICUT_FACTORS_MESSAGES_HXX
