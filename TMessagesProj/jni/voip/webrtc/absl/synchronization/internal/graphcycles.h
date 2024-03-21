// Copyright 2017 The Abseil Authors.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//      https://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
//

#ifndef ABSL_SYNCHRONIZATION_INTERNAL_GRAPHCYCLES_H_
#define ABSL_SYNCHRONIZATION_INTERNAL_GRAPHCYCLES_H_

// graph that is being built up incrementally.
//
// Nodes are identified by small integers.  It is not possible to
// record multiple edges with the same (source, destination) pair;
// requests to add an edge where one already exists are silently
// ignored.
//
// It is also not possible to introduce a cycle; an attempt to insert
// an edge that would introduce a cycle fails and returns false.
//
// GraphCycles uses no internal locking; calls into it should be
// serialized externally.

//   Works well on sparse graphs, poorly on dense graphs.
//   Extra information is maintained incrementally to detect cycles quickly.
//   InsertEdge() is very fast when the edge already exists, and reasonably fast
//   otherwise.
//   FindPath() is linear in the size of the graph.
// The current implementation uses O(|V|+|E|) space.

#include <cstdint>

#include "absl/base/config.h"

namespace absl {
ABSL_NAMESPACE_BEGIN
namespace synchronization_internal {

struct GraphId {
  uint64_t handle;

  bool operator==(const GraphId& x) const { return handle == x.handle; }
  bool operator!=(const GraphId& x) const { return handle != x.handle; }
};

inline GraphId InvalidGraphId() {
  return GraphId{0};
}

class GraphCycles {
 public:
  GraphCycles();
  ~GraphCycles();



  GraphId GetId(void* ptr);


  void RemoveNode(void* ptr);


  void* Ptr(GraphId id);



  bool InsertEdge(GraphId source_node, GraphId dest_node);

  void RemoveEdge(GraphId source_node, GraphId dest_node);

  bool HasNode(GraphId node);

  bool HasEdge(GraphId source_node, GraphId dest_node) const;


  bool IsReachable(GraphId source_node, GraphId dest_node) const;












  int FindPath(GraphId source, GraphId dest, int max_path_len,
               GraphId path[]) const;





  void UpdateStackTrace(GraphId id, int priority,
                        int (*get_stack_trace)(void**, int));


  int GetStackTrace(GraphId id, void*** ptr);


  bool CheckInvariants() const;

  struct Rep;
 private:
  Rep *rep_;      // opaque representation
  GraphCycles(const GraphCycles&) = delete;
  GraphCycles& operator=(const GraphCycles&) = delete;
};

}  // namespace synchronization_internal
ABSL_NAMESPACE_END
}  // namespace absl

#endif
