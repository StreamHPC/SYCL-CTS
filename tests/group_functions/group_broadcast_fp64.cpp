/*******************************************************************************
//
//  SYCL 2020 Conformance Test Suite
//
//  Copyright (c) 2023 The Khronos Group Inc.
//
//  Licensed under the Apache License, Version 2.0 (the "License");
//  you may not use this file except in compliance with the License.
//  You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
//  Unless required by applicable law or agreed to in writing, software
//  distributed under the License is distributed on an "AS IS" BASIS,
//  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
//  See the License for the specific language governing permissions and
//  limitations under the License.
//
*******************************************************************************/

#include "group_broadcast.h"

TEMPLATE_TEST_CASE_SIG("Group broadcast", "[group_func][fp64][dim]",
                       ((int D), D), 1, 2, 3) {
  auto queue = sycl_cts::util::get_cts_object::queue();

  if (queue.get_device().has(sycl::aspect::fp64)) {
    broadcast_group<D, double>(queue);
  } else {
    WARN("Device does not support double precision floating point operations.");
  }
}

TEMPLATE_TEST_CASE_SIG("Sub-group broadcast and select",
                       "[group_func][fp64][dim]", ((int D), D), 1, 2, 3) {
  auto queue = sycl_cts::util::get_cts_object::queue();

  if (queue.get_device().has(sycl::aspect::fp64)) {
    broadcast_sub_group<D, double>(queue);
  } else {
    WARN("Device does not support double precision floating point operations.");
  }
}
