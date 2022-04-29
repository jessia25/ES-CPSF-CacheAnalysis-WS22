#ifndef ABSSTATE_H
#define ABSSTATE_H

#include <cassert>
#include <cstddef>
#include <cstdlib>
#include <iostream>
#include <list>
#include <map>
#include <ostream>
#include <sstream>
#include <string>

#include <llvm/Support/raw_ostream.h>

#include "Address.h"
// Forward declarations

namespace cacheAnaPass {
class AbstractState;

} // namespace cacheAnaPass

class AbstractState {
public: // everything is public, because IDGAF
  std::list<unsigned int> Successors;
  std::list<unsigned int> Predecessors;

  unsigned int Addr;
  unsigned int Unrolled;

  bool Computed = false;

  // Only entries below this comment are needed for the exercise.

  /**
   * @brief Containing all Abstract Cache Tags.
   *        Key of the list has no Meaning.
   *
   */
  struct Entry {
    std::list<unsigned int> Blocks;
  };

  /**
   * @brief Cache Set, Key is the Age of the Entries.
   *
   */
  struct Set {
    // uInt in this map is the Age.
    std::map<unsigned int, Entry> Associativity;
  };

  /**
   * @brief Cache Sets, key is the Set number [0-15], derived from Address.
   *
   */
  std::map<unsigned int, Set> Sets;

  AbstractState(AbstractState const &Copy) {
    Addr = Copy.Addr;
    Unrolled = Copy.Unrolled;
    for (auto S : Copy.Sets) {
      unsigned int SetNr = S.first;
      for (auto E : S.second.Associativity) {
        unsigned int Age = E.first;
        for (auto B : E.second.Blocks) {
          Sets[SetNr].Associativity[Age].Blocks.push_back(B);
        }
      }
    }
  }

  AbstractState() {}

  AbstractState(unsigned int AddressIn) {
    Addr = AddressIn;
    Unrolled = 0;
  }

  AbstractState(unsigned int AddressIn, unsigned int UnrolledIn) {
    Addr = AddressIn;
    Unrolled = UnrolledIn;
  }

  // AbstractState(Address Addr) {
  //   Sets[Addr.Index].Associativity[0] = {{Addr.Tag}};
  // }

  void setUnrolled(unsigned int In) { Unrolled = In; }

  /**
   * @brief Executes an Must LRU Join on the AbstractCacheState
   *
   * @param In, AbstractState that gets joined into the State.
   */
  void mustJoin(AbstractState In) {
    /**
     * The exercise is to Fill this function with an LRU must Join.
     * For this you need to use Sets. Associativity and Blocks.
     */
  }

  /**
   * @brief Checks if Address Addr is in Cache
   *
   * @param Addr Address to check.
   * @return true CacheState contains Address Addr
   * @return false CacheState does not contain Address Addr
   */
  bool isHit(Address Addr) {
    for (auto E : Sets[Addr.Index].Associativity) {
      for (auto B : E.second.Blocks) {
        if (B == Addr.Tag)
          return true;
      }
    }
    return false;
  }

  /**
   * @brief Updates the AbstractState with given Address
   *
   * @param Addr , Address
   */
  void update(Address Addr) {
    // This loopages all entries by one. 3 <-2, 2<-1, 1<-0
    for (int I = 3; I > 0; I--) {
      Sets[Addr.Index].Associativity[I] = Sets[Addr.Index].Associativity[I - 1];
    }
    // entry at age 0 is updated with current address.
    Sets[Addr.Index].Associativity[0].Blocks = {Addr.Tag};
  }

  /**
   * @brief Updates the AbstractState with given AbstractState
   *
   * @param UpdateState, State that gets merged into State with Age+1.
   */
  void update(AbstractState UpdateState) {
    for (auto S : UpdateState.Sets) {
      unsigned int Index = S.first;
      for (auto E : S.second.Associativity) {
        unsigned int Age = E.first;
        // If updated age is greater 4 The Tag is no longer in Cache.
        // Due to associativity of 4 per set.
        if (Age >= 4)
          break;
        for (auto B : E.second.Blocks) {
          Sets[Index].Associativity[Age].Blocks.push_back(B);
        }
      }
    }
  }

  /**
   * @brief Fills the AbstractState PreState and updates with PreAddress.
   *
   * @param PreState, State that fills this state.
   *
   * @param PreAddr Address of PreState
   */
  void fill(AbstractState PreState, Address PreAddr) {
    // copy Pre State into this.
    for (auto S : PreState.Sets) {
      unsigned int Index = S.first;
      for (auto E : S.second.Associativity) {
        unsigned int Age = E.first;
        // If updated age is greater 4 The Tag is no longer in Cache.
        // Due to associativity of 4 per set.
        if (Age >= 4)
          break;
        for (auto B : E.second.Blocks) {
          Sets[Index].Associativity[Age].Blocks.push_back(B);
        }
      }
    }
    // update this with PreAddr
    this->update(PreAddr);
  }

  void dump() {
    llvm::outs() << Addr << " {\n";
    llvm::outs() << "Unrolled: " << Unrolled << "\n";
    llvm::outs() << "Predecessors: ";
    for (auto PreNr : Predecessors) {
      llvm::outs() << PreNr << " ";
    }
    llvm::outs() << "\n";

    llvm::outs() << "Successors: ";
    for (auto SuccNr : Successors) {
      llvm::outs() << SuccNr << " ";
    }
    llvm::outs() << "\n";

    for (auto SetPair : Sets) {
      llvm::outs() << "Set[" << SetPair.first << "]: \n";
      for (auto EntryPair : SetPair.second.Associativity) {
        llvm::outs() << "  Age[" << EntryPair.first << "]: ";
        for (auto Block : EntryPair.second.Blocks) {
          llvm::outs() << Block << " ";
        }
        llvm::outs() << "\n";
      }
    }
    llvm::outs() << "}\n";
  }

};     // namespace
#endif // STATE_H