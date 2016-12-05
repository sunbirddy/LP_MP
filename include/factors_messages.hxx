#ifndef LP_MP_FACTORS_MESSAGES_HXX
#define LP_MP_FACTORS_MESSAGES_HXX

#include <tuple>
#include <array>
#include <iostream>
#include <numeric>
#include <algorithm>
#include <functional>
#include <utility>
#include <memory>
#include <limits>
#include <exception>
#include <typeinfo>
#include <type_traits>
#include <assert.h>
#include <cxxabi.h>

#include "template_utilities.hxx"
#include "function_existence.hxx"
#include "meta/meta.hpp"
#include "static_if.hxx"
#include "MemoryPool.h"

#include "memory_allocator.hxx"

#include "LP_MP.h"

// do zrobienia: remove these
//#include "factors/reparametrization_storage.hxx"  // also delete file
#include "messages/message_storage.hxx"
#include <fstream>
#include <sstream>
#include <valarray> // do zrobienia: do not use
#include <vector>

// this file provides message and factor containers. The factors and messages are plugged into the container and then every method call is dispatched correctly with static polymorphism and template tricks.

// do zrobienia: Introduce MessageConstraint and FactorConstraint for templates
// cleanup name inconsistencies: MessageType, MessageDispatcher etc

namespace LP_MP {

// we must check existence of functions in message classes. The necessary test code is concentrated here. 
namespace FunctionExistence {

// Macros to construct help functions for checking existence of member functions of classes
LP_MP_FUNCTION_EXISTENCE_CLASS(HasReceiveMessageFromRight,ReceiveMessageFromRight);
LP_MP_FUNCTION_EXISTENCE_CLASS(HasReceiveMessageFromLeft, ReceiveMessageFromLeft);
   
LP_MP_FUNCTION_EXISTENCE_CLASS(HasReceiveRestrictedMessageFromRight,ReceiveRestrictedMessageFromRight);
LP_MP_FUNCTION_EXISTENCE_CLASS(HasReceiveRestrictedMessageFromLeft, ReceiveRestrictedMessageFromLeft);

LP_MP_FUNCTION_EXISTENCE_CLASS(HasSendMessageToRight,SendMessageToRight);
LP_MP_FUNCTION_EXISTENCE_CLASS(HasSendMessageToLeft, SendMessageToLeft);

LP_MP_FUNCTION_EXISTENCE_CLASS(HasSendMessagesToRight,SendMessagesToRight);
LP_MP_FUNCTION_EXISTENCE_CLASS(HasSendMessagesToLeft, SendMessagesToLeft);

LP_MP_FUNCTION_EXISTENCE_CLASS(HasRepamRight, RepamRight);
LP_MP_FUNCTION_EXISTENCE_CLASS(HasRepamLeft, RepamLeft);

LP_MP_FUNCTION_EXISTENCE_CLASS(HasComputeLeftFromRightPrimal, ComputeLeftFromRightPrimal);
LP_MP_FUNCTION_EXISTENCE_CLASS(HasComputeRightFromLeftPrimal, ComputeRightFromLeftPrimal); 

LP_MP_FUNCTION_EXISTENCE_CLASS(HasCheckPrimalConsistency, CheckPrimalConsistency); 

LP_MP_FUNCTION_EXISTENCE_CLASS(HasPrimalSize,PrimalSize);
LP_MP_FUNCTION_EXISTENCE_CLASS(HasPropagatePrimal, PropagatePrimal);
LP_MP_FUNCTION_EXISTENCE_CLASS(HasMaximizePotential, MaximizePotential);

LP_MP_FUNCTION_EXISTENCE_CLASS(HasCreateConstraints, CreateConstraints);
LP_MP_FUNCTION_EXISTENCE_CLASS(HasGetNumberOfAuxVariables, GetNumberOfAuxVariables);
LP_MP_FUNCTION_EXISTENCE_CLASS(HasReduceLp, ReduceLp);

LP_MP_ASSIGNMENT_FUNCTION_EXISTENCE_CLASS(IsAssignable, operator[]);
}

// function getters for statically dispatching ReceiveMessage and SendMessage to left and right side correctly, used in FactorContainer
template<typename MSG_CONTAINER>
struct LeftMessageFuncGetter
{
   using ConnectedFactorType = typename MSG_CONTAINER::RightFactorContainer;

   //constexpr static decltype(&MSG_CONTAINER::GetLeftMessage) GetMessageFunc() { return &MSG_CONTAINER::GetLeftMessage; }

   constexpr static decltype(&MSG_CONTAINER::ReceiveMessageFromRightContainer) GetReceiveFunc() { return &MSG_CONTAINER::ReceiveMessageFromRightContainer; }
   constexpr static decltype(&MSG_CONTAINER::ReceiveRestrictedMessageFromRightContainer) GetReceiveRestrictedFunc() { return &MSG_CONTAINER::ReceiveRestrictedMessageFromRightContainer; }
   constexpr static decltype(&MSG_CONTAINER::SendMessageToRightContainer) GetSendFunc() { return &MSG_CONTAINER::SendMessageToRightContainer; }

   template<typename LEFT_FACTOR, typename MSG_ARRAY, typename ITERATOR>
   constexpr static decltype(&MSG_CONTAINER::template SendMessagesToRightContainer<LEFT_FACTOR, MSG_ARRAY, ITERATOR>) GetSendMessagesFunc() 
   { return &MSG_CONTAINER::template SendMessagesToRightContainer<LEFT_FACTOR, MSG_ARRAY, ITERATOR>; }

   constexpr static bool 
   CanCallReceiveMessage()
   { return MSG_CONTAINER::CanCallReceiveMessageFromRightContainer(); }

   constexpr static bool
   CanCallReceiveRestrictedMessage()
   { return MSG_CONTAINER::CanCallReceiveRestrictedMessageFromRightContainer(); }

   constexpr static bool CanCallSendMessage() 
   { return MSG_CONTAINER::CanCallSendMessageToRightContainer(); }

   // do zrobienia: Is LEFT_FACTOR parameter needed at all? same for RightMessageFuncGetter
   template<typename LEFT_FACTOR, typename MSG_ARRAY, typename ITERATOR>
   constexpr static bool 
   CanCallSendMessages()
   { return MSG_CONTAINER::template CanCallSendMessagesToRightContainer<LEFT_FACTOR, MSG_ARRAY, ITERATOR>(); }

   // do zrobienia: rename CanPropagatePrimalThroughMessage
   constexpr static bool CanComputePrimalThroughMessage()
   { return MSG_CONTAINER::CanComputeRightFromLeftPrimal(); }
   constexpr static decltype(&MSG_CONTAINER::ComputeRightFromLeftPrimal) GetComputePrimalThroughMessageFunc()
   { return &MSG_CONTAINER::ComputeRightFromLeftPrimal; }

   constexpr static Chirality Chirality() { return Chirality::left; }
   constexpr static bool factor_holds_messages() { return MSG_CONTAINER::left_factor_holds_messages(); }
};

template<typename MSG_CONTAINER>
struct RightMessageFuncGetter
{
   using ConnectedFactorType = typename MSG_CONTAINER::LeftFactorContainer;

   //constexpr static decltype(&MSG_CONTAINER::GetRightMessage) GetMessageFunc() { return &MSG_CONTAINER::GetRightMessage; }

   constexpr static decltype(&MSG_CONTAINER::ReceiveMessageFromLeftContainer) GetReceiveFunc() { return &MSG_CONTAINER::ReceiveMessageFromLeftContainer; }
   constexpr static decltype(&MSG_CONTAINER::ReceiveRestrictedMessageFromLeftContainer) GetReceiveRestrictedFunc() { return &MSG_CONTAINER::ReceiveRestrictedMessageFromLeftContainer; }
   constexpr static decltype(&MSG_CONTAINER::SendMessageToLeftContainer) GetSendFunc() { return &MSG_CONTAINER::SendMessageToLeftContainer; }

   template<typename RIGHT_FACTOR, typename MSG_ARRAY, typename ITERATOR>
   constexpr static decltype(&MSG_CONTAINER::template SendMessagesToLeftContainer<RIGHT_FACTOR, MSG_ARRAY, ITERATOR>) GetSendMessagesFunc() 
   { return &MSG_CONTAINER::template SendMessagesToLeftContainer<RIGHT_FACTOR, MSG_ARRAY, ITERATOR>; }

   constexpr static bool CanCallReceiveMessage() 
   { return MSG_CONTAINER::CanCallReceiveMessageFromLeftContainer(); }

   constexpr static bool CanCallReceiveRestrictedMessage() 
   { return MSG_CONTAINER::CanCallReceiveRestrictedMessageFromLeftContainer(); }

   constexpr static bool CanCallSendMessage() 
   { return MSG_CONTAINER::CanCallSendMessageToLeftContainer(); }

   template<typename RIGHT_FACTOR, typename MSG_ARRAY, typename ITERATOR>
   constexpr static bool
   CanCallSendMessages()
   { return MSG_CONTAINER::template CanCallSendMessagesToLeftContainer<RIGHT_FACTOR, MSG_ARRAY, ITERATOR>(); }

   constexpr static bool CanComputePrimalThroughMessage()
   { return MSG_CONTAINER::CanComputeLeftFromRightPrimal(); }
   constexpr static decltype(&MSG_CONTAINER::ComputeLeftFromRightPrimal) GetComputePrimalThroughMessageFunc()
   { return &MSG_CONTAINER::ComputeLeftFromRightPrimal; }

   constexpr static Chirality Chirality() { return Chirality::right; }
   constexpr static bool factor_holds_messages() { return MSG_CONTAINER::right_factor_holds_messages(); }
};

template<class MSG_CONTAINER, template<typename> class FuncGetter>
struct MessageDispatcher
{
   using ConnectedFactorType = typename FuncGetter<MSG_CONTAINER>::ConnectedFactorType; // this is the type of factor container to which the message is connected

   constexpr static bool CanCallReceiveMessage() { return FuncGetter<MSG_CONTAINER>::CanCallReceiveMessage(); }
   static void ReceiveMessage(MSG_CONTAINER& t)
   {
      auto staticMemberFunc = FuncGetter<MSG_CONTAINER>::GetReceiveFunc();
      return (t.*staticMemberFunc)();
   }
   constexpr static bool CanCallReceiveRestrictedMessage() { return FuncGetter<MSG_CONTAINER>::CanCallReceiveRestrictedMessage(); }
   static void ReceiveRestrictedMessage(MSG_CONTAINER& t, PrimalSolutionStorage::Element primal)
   {
      auto staticMemberFunc = FuncGetter<MSG_CONTAINER>::GetReceiveRestrictedFunc();
      return (t.*staticMemberFunc)(primal);
   }

   // individual message sending
   constexpr static bool CanCallSendMessage() { return FuncGetter<MSG_CONTAINER>::CanCallSendMessage(); }

   template<typename FACTOR_TYPE>
   static void SendMessage(FACTOR_TYPE* f, MSG_CONTAINER& t, const REAL omega)
   {
      auto staticMemberFunc = FuncGetter<MSG_CONTAINER>::GetSendFunc();
      return (t.*staticMemberFunc)(f, omega);
   }

   // batch message sending
   template<typename FACTOR, typename MSG_ARRAY, typename ITERATOR>
   constexpr static bool CanCallSendMessages() { return FuncGetter<MSG_CONTAINER>::template CanCallSendMessages<FACTOR, MSG_ARRAY, ITERATOR>(); }

   template<typename FACTOR, typename MSG_ARRAY, typename ITERATOR>
   static void SendMessages(const FACTOR& f, const MSG_ARRAY& msgs, ITERATOR omegaBegin)
   {
      auto staticMemberFunc = FuncGetter<MSG_CONTAINER>::template GetSendMessagesFunc<FACTOR, MSG_ARRAY, ITERATOR>();
      (*staticMemberFunc)(f, msgs, omegaBegin);
   }

   //static REAL GetMessage(MSG_CONTAINER& t, const INDEX i)
   //{
   //   auto staticMemberFunc = FuncGetter<MSG_CONTAINER>::GetMessageFunc();
   //   return (t.*staticMemberFunc)(i);
   //}

   constexpr static bool CanComputePrimalThroughMessage() // do zrobienia: return false, if the factor from which this is called computes its own primal already
   {
      return FuncGetter<MSG_CONTAINER>::CanComputePrimalThroughMessage();
   }

   static void ComputePrimalThroughMessage(MSG_CONTAINER& t, typename PrimalSolutionStorage::Element primal) 
   {
      auto staticMemberFunc = FuncGetter<MSG_CONTAINER>::GetComputePrimalThroughMessageFunc();
      return (t.*staticMemberFunc)(primal);
   }
   constexpr static Chirality Chirality() { return FuncGetter<MSG_CONTAINER>::Chirality(); }
   constexpr static bool factor_holds_messages() { return FuncGetter<MSG_CONTAINER>::factor_holds_messages(); }
};

template<INDEX NO_ELEMENTS, typename T>
class FixedSizeMessageContainer : public std::array<T*,NO_ELEMENTS> {
public: 
   FixedSizeMessageContainer() { this->fill(nullptr); }
   ~FixedSizeMessageContainer() {}
   void push_back(T* t) {
      // do zrobienia: possibly use binary search when NO_ELEMENTS is bigger than some threshold
      for(INDEX i=0; i<NO_ELEMENTS; ++i) {
         if(this->operator[](i) == nullptr) {
            this->operator[](i) = t;
            return;
         }
      }
      throw std::range_error("added more messages than can be held");
   }
   constexpr INDEX size() const { return NO_ELEMENTS; }
};

// holds at most NO_ELEMENTS in std::array. Unused entries have nullptr in them
template<INDEX NO_ELEMENTS, typename T>
class UpToFixedSizeMessageContainer : public std::array<T*,NO_ELEMENTS> {
public:
   UpToFixedSizeMessageContainer() : size_(0) { this->fill(nullptr); }
   ~UpToFixedSizeMessageContainer() { 
      static_assert(NO_ELEMENTS > 0, "");
   }
   void push_back(T* t) {
      assert(size_ < NO_ELEMENTS);
      this->operator[](size_) = t;
      ++size_;
   }
   INDEX size() const { return size_; }
   auto end() const -> decltype(this->end()) { return this->begin() + size(); }


private:
   unsigned char size_;
};

// for one element we do not need to store its size explicitly
template<typename T>
class UpToFixedSizeMessageContainer<1,T> : public std::array<T*,1> {
public:
   UpToFixedSizeMessageContainer() { this->fill(nullptr); }
   void push_back(T* t) {
      assert((*this)[0] == nullptr);
      (*this)[0] = t;
   }
   INDEX size() const { return (*this)[0] == nullptr ? 0 : 1; } 
   auto end() const -> decltype(this->end()) { return this->begin() + size(); }
};

template<typename T>
class UpToFixedSizeMessageContainer<2,T> : public std::array<T*,2> {
public:
   UpToFixedSizeMessageContainer() { this->fill(nullptr); }
   void push_back(T* t) {
      if((*this)[0] == nullptr) {
         (*this)[0] = t; 
      } else if((*this)[1] == nullptr) {
         (*this)[1] = t; 
      } else {
         assert(false);
      }
   }
   INDEX size() const {
      assert(false);
      return ((*this)[0] != nullptr)*1 + ((*this)[1] != nullptr)*1;
      //if((*this)[0] == nullptr) {
      //   (*this)[0] = t; 
      //} else if((*this)[1] == nullptr) {
      //   (*this)[1] = t; 
      //} else {
      //return 2; }
      //} 
}
   auto end() const -> decltype(this->end()) { return this->begin() + size(); }
};

template<typename M, Chirality CHIRALITY>
class VariableSizeMessageContainerIterator {
public:
   VariableSizeMessageContainerIterator(M* m) : m_(m) {}
   VariableSizeMessageContainerIterator operator++() {
      if(CHIRALITY == Chirality::right) {
         m_ = m_->next_right_message_container::next_msg();
      } else {
         m_ = m_->next_left_message_container::next_msg();
      }
      return *this;
   }
   M* operator*() const { return m_; } 
   bool operator==(const VariableSizeMessageContainerIterator& o) const { return m_ == o.m_; }
   bool operator!=(const VariableSizeMessageContainerIterator& o) const { return m_ != o.m_; }
private:
   M* m_;
};

template<typename M, Chirality CHIRALITY>
class VariableSizeMessageContainer
{
public:
   VariableSizeMessageContainer() : m_(nullptr), size_(0) {}
   INDEX size() const { return size_; }
   void push_back(M* m) { // actually it is push_front
      if(CHIRALITY == Chirality::right) {
         m->next_right_message_container::next_msg(m_);
      } else {
         m->next_left_message_container::next_msg(m_);
      }
      m_ = m;
      ++size_;
   }
   VariableSizeMessageContainerIterator<M,CHIRALITY> begin() const {
      return VariableSizeMessageContainerIterator<M,CHIRALITY>(m_);
   }
   VariableSizeMessageContainerIterator<M,CHIRALITY> end() const {
      return VariableSizeMessageContainerIterator<M,CHIRALITY>(nullptr);
   }

private:
   M* m_;
   INDEX size_;
};

// N=0 means variable number of messages, > 0 means compile time fixed number of messages and <0 means at most compile time number of messages
// see config.hxx for shortcuts
template<SIGNED_INDEX N, typename MESSAGE_CONTAINER_TYPE, Chirality CHIRALITY>
struct MessageContainerSelector {
   using type = 
      typename std::conditional<(N > 0), FixedSizeMessageContainer<INDEX(N),MESSAGE_CONTAINER_TYPE>,
      typename std::conditional<(N < 0), UpToFixedSizeMessageContainer<INDEX(-N),MESSAGE_CONTAINER_TYPE>, 
                                         VariableSizeMessageContainer<MESSAGE_CONTAINER_TYPE,CHIRALITY> >::type >::type;
};

template<typename MSG_CONTAINER, bool HOLD>
struct next_left_message_container {
   MSG_CONTAINER* next_msg() const { assert(false); return nullptr; }
   void next_msg(MSG_CONTAINER*) { assert(false); }
};

template<typename MSG_CONTAINER>
struct next_left_message_container<MSG_CONTAINER,true> {
   void next_msg(MSG_CONTAINER* m) { next = m; }
   MSG_CONTAINER* next_msg() const { return next; }
   MSG_CONTAINER* next = nullptr;
};
   
template<typename MSG_CONTAINER, bool HOLD>
struct next_right_message_container {
   MSG_CONTAINER* next_msg() const { assert(false); return nullptr; }
   void next_msg(MSG_CONTAINER*) { assert(false); }
};

template<typename MSG_CONTAINER>
struct next_right_message_container<MSG_CONTAINER,true> {
   void next_msg(MSG_CONTAINER* m) { next = m; }
   MSG_CONTAINER* next_msg() const { return next; }
   MSG_CONTAINER* next = nullptr;
};



// Class holding message and left and right factor
// do zrobienia: possibly replace {LEFT|RIGHT}_FACTOR_NO by their type
template<typename MESSAGE_TYPE, 
         INDEX LEFT_FACTOR_NO, INDEX RIGHT_FACTOR_NO, SIGNED_INDEX NO_OF_LEFT_FACTORS, SIGNED_INDEX NO_OF_RIGHT_FACTORS,
         typename FACTOR_MESSAGE_TRAIT, 
         INDEX MESSAGE_NO
         >
class MessageContainer : //public MessageStorageSelector<MESSAGE_SIZE,true>::type, 
                           public MessageTypeAdapter
                         // when NO_OF_LEFT_FACTORS is zero, we hold factors in linked list
                         ,public next_left_message_container<MessageContainer<MESSAGE_TYPE,LEFT_FACTOR_NO,RIGHT_FACTOR_NO,NO_OF_LEFT_FACTORS,NO_OF_RIGHT_FACTORS,FACTOR_MESSAGE_TRAIT,MESSAGE_NO>,NO_OF_LEFT_FACTORS == 0> 
                         ,public next_right_message_container<MessageContainer<MESSAGE_TYPE,LEFT_FACTOR_NO,RIGHT_FACTOR_NO,NO_OF_LEFT_FACTORS,NO_OF_RIGHT_FACTORS,FACTOR_MESSAGE_TRAIT,MESSAGE_NO>,NO_OF_RIGHT_FACTORS == 0>
{
public:
   using leftFactorNumber_t = std::integral_constant<INDEX, LEFT_FACTOR_NO>;
   static constexpr INDEX leftFactorNumber = LEFT_FACTOR_NO;
   static constexpr INDEX rightFactorNumber = RIGHT_FACTOR_NO;

   using MessageContainerType = MessageContainer<MESSAGE_TYPE, LEFT_FACTOR_NO, RIGHT_FACTOR_NO, NO_OF_LEFT_FACTORS, NO_OF_RIGHT_FACTORS, FACTOR_MESSAGE_TRAIT, MESSAGE_NO>;
   using MessageType = MESSAGE_TYPE;
   //typedef typename MessageStorageSelector<MESSAGE_SIZE,true>::type MessageStorageType; // do zrobienia: true is just for now. In general, message need not hold actual message, except when some factor is reparametrized implicitly

   // structures used in FactorContainer to hold pointers to messages
   using LeftMessageContainerStorageType = typename MessageContainerSelector<NO_OF_LEFT_FACTORS, MessageContainerType, Chirality::left>::type;
   using RightMessageContainerStorageType = typename MessageContainerSelector<NO_OF_RIGHT_FACTORS, MessageContainerType, Chirality::right>::type;

   // FactorContainer
   using LeftFactorContainer = meta::at_c<typename FACTOR_MESSAGE_TRAIT::FactorList, leftFactorNumber>;
   using RightFactorContainer = meta::at_c<typename FACTOR_MESSAGE_TRAIT::FactorList, rightFactorNumber>;
   // Factor
   using LeftFactorType = typename LeftFactorContainer::FactorType;
   using RightFactorType = typename RightFactorContainer::FactorType;

   constexpr static bool left_factor_holds_messages() { return NO_OF_LEFT_FACTORS != 0; }
   constexpr static bool right_factor_holds_messages() { return NO_OF_RIGHT_FACTORS != 0; }
   
   template<typename ...ARGS>
   MessageContainer(ARGS... args, LeftFactorContainer* const l, RightFactorContainer* const r) 
   : msg_op_(args...),
   leftFactor_(l),
   rightFactor_(r)
   {
      leftFactor_->template AddMessage<MessageDispatcher<MessageContainerType, LeftMessageFuncGetter>, MessageContainerType>(this);
      rightFactor_->template AddMessage<MessageDispatcher<MessageContainerType, RightMessageFuncGetter>, MessageContainerType>(this);
   }

   MessageContainer(MESSAGE_TYPE msg_op, LeftFactorContainer* const l, RightFactorContainer* const r) 
      ://MessageStorageType(),
      msg_op_(msg_op),
      leftFactor_(l),
      rightFactor_(r)
   {
      leftFactor_->template AddMessage<MessageDispatcher<MessageContainerType, LeftMessageFuncGetter>, MessageContainerType>(this);
      rightFactor_->template AddMessage<MessageDispatcher<MessageContainerType, RightMessageFuncGetter>, MessageContainerType>(this);
      int status;
      //std::cout << "msg holding type = " << abi::__cxa_demangle(typeid(*this).name(),0,0,&status) << "\n";
      //std::cout << FunctionExistence::IsAssignable<RightFactorContainer,REAL,INDEX>() << "\n";
      //std::cout << "msg holding type = " << abi::__cxa_demangle(typeid(msg_op_).name(),0,0,&status) << "\n";
      //std::cout << "left factor number = " << leftFactorNumber << "\n";
      //std::cout << "right factor number = " << rightFactorNumber << "\n";
      //std::cout << "left factor type = " << abi::__cxa_demangle(typeid(LeftFactorContainer).name(),0,0,&status) << "\n";
      //std::cout << "right factor type = " << abi::__cxa_demangle(typeid(RightFactorContainer).name(),0,0,&status) << "\n";
      // register messages in factors
   }
   ~MessageContainer() {
      static_assert(meta::unique<typename FACTOR_MESSAGE_TRAIT::MessageList>::size() == FACTOR_MESSAGE_TRAIT::MessageList::size(), 
            "Message list must have unique elements");
      static_assert(MESSAGE_NO >= 0 && MESSAGE_NO < FACTOR_MESSAGE_TRAIT::MessageList::size(), "message number must be smaller than length of message list");
      static_assert(leftFactorNumber < FACTOR_MESSAGE_TRAIT::FactorList::size(), "left factor number out of bound");
      static_assert(rightFactorNumber < FACTOR_MESSAGE_TRAIT::FactorList::size(), "right factor number out of bound");
      // do zrobienia: put message constraint here, i.e. which methods MESSAGE_TYPE must minimally implement
   } 
   
   // overloaded new so that factor containers are allocated by global block allocator consecutively
   void* operator new(std::size_t size)
   {
      assert(size == sizeof(MessageContainerType));
      //return (void*) global_real_block_allocator.allocate(size/sizeof(REAL),1);
      return Allocator::get().allocate(1);
   }
   void operator delete(void* mem)
   {
      Allocator::get().deallocate((MessageContainerType*) mem);
      //assert(false);
      //global_real_block_allocator.deallocate(mem,sizeof(FactorContainerType));
   }



   constexpr static bool
   CanCallReceiveMessageFromRightContainer()
   { 
      return FunctionExistence::HasReceiveMessageFromRight<MessageType, void, 
      RightFactorType, MessageContainerType>(); 
   }
   void ReceiveMessageFromRightContainer()
   { msg_op_.ReceiveMessageFromRight(*rightFactor_->GetFactor(), *static_cast<MessageContainerView<Chirality::right>*>(this) ); }

   // do zrobienia: must use one additional argument for primal storage
   constexpr static bool
   CanCallReceiveRestrictedMessageFromRightContainer()
   { 
      return FunctionExistence::HasReceiveRestrictedMessageFromRight<MessageType, void, 
      RightFactorType, MessageContainerType, PrimalSolutionStorage::Element>(); // do zrobienia: signature is slighly different: MessageContainerType is not actually used
   }
   void ReceiveRestrictedMessageFromRightContainer(PrimalSolutionStorage::Element primal)
   {
     //assert(rightFactor_->GetPrimalOffset() + rightFactor_->PrimalSize() <= primal.size());
      msg_op_.ReceiveRestrictedMessageFromRight(*(rightFactor_->GetFactor()), *static_cast<OneSideMessageContainerView<Chirality::left>*>(this), primal + rightFactor_->GetPrimalOffset());
   }

   constexpr static bool 
   CanCallReceiveMessageFromLeftContainer()
   { 
      return FunctionExistence::HasReceiveMessageFromLeft<MessageType, void, 
      LeftFactorType, MessageContainerType>(); 
   }
   void ReceiveMessageFromLeftContainer()
   { msg_op_.ReceiveMessageFromLeft(*(leftFactor_->GetFactor()), *static_cast<MessageContainerView<Chirality::left>*>(this) ); }

   constexpr static bool
   CanCallReceiveRestrictedMessageFromLeftContainer()
   { 
      return FunctionExistence::HasReceiveRestrictedMessageFromLeft<MessageType, void, 
      LeftFactorType, MessageContainerType, PrimalSolutionStorage::Element>(); 
   }
   void ReceiveRestrictedMessageFromLeftContainer(PrimalSolutionStorage::Element primal)
   {
     //assert(leftFactor_->GetPrimalOffset() + leftFactor_->PrimalSize() <= primal.size());
      msg_op_.ReceiveRestrictedMessageFromLeft(*(leftFactor_->GetFactor()), *static_cast<OneSideMessageContainerView<Chirality::right>*>(this), primal + leftFactor_->GetPrimalOffset());
   }


   constexpr static bool 
   CanCallSendMessageToRightContainer()
   { 
      return FunctionExistence::HasSendMessageToRight<MessageType, void, 
      LeftFactorType, MessageContainerType, REAL>(); 
   }

   void SendMessageToRightContainer(LeftFactorType* l, const REAL omega)
   {
      //msg_op_.SendMessageToRight(leftFactor_->GetFactor(), *static_cast<MessageContainerView<Chirality::left>*>(this), omega);
      msg_op_.SendMessageToRight(*l, *static_cast<MessageContainerView<Chirality::left>*>(this), omega);
   }

   constexpr static bool
   CanCallSendMessageToLeftContainer()
   { 
      return FunctionExistence::HasSendMessageToLeft<MessageType, void, 
      RightFactorType, MessageContainerType, REAL>(); 
   }

   void SendMessageToLeftContainer(RightFactorType* r, const REAL omega)
   {
      //msg_op_.SendMessageToLeft(rightFactor_->GetFactor(), *static_cast<MessageContainerView<Chirality::right>*>(this), omega);
      msg_op_.SendMessageToLeft(*r, *static_cast<MessageContainerView<Chirality::right>*>(this), omega);
   }

   template<typename RIGHT_FACTOR, typename MSG_ARRAY, typename ITERATOR>
   constexpr static bool
   CanCallSendMessagesToLeftContainer()
   { 
      return FunctionExistence::HasSendMessagesToLeft<MessageType, void, RIGHT_FACTOR, MSG_ARRAY, MSG_ARRAY, ITERATOR>();
   }
   template<typename RIGHT_FACTOR, typename MSG_ARRAY, typename ITERATOR>
   static void SendMessagesToLeftContainer(const RIGHT_FACTOR& rightFactor, const MSG_ARRAY& msgs, ITERATOR omegaBegin) 
   {
      // this is not nice: heavy static casting!
      // We get msgs an array with pointers to messages. We wrap it so that operator[] gives a reference to the respective message with the correct view
      struct MessageIterator {
         using type = decltype(msgs.begin());
         MessageIterator(type it) : it_(it) {}
         MessageContainerView<Chirality::right>& operator*() const {
            return *(static_cast<MessageContainerView<Chirality::right>*>( *it_ )); 
         }
         MessageIterator operator++() {
            ++it_;
            return *this;
         }
         bool operator!=(const MessageIterator& o) const {
            return it_ != o.it_; 
         }
         private:
         type it_;
      };
      return MessageType::SendMessagesToLeft(rightFactor->GetFactor(), MessageIterator(msgs.begin()), MessageIterator(msgs.end()), omegaBegin);

      //struct ViewWrapper : public MSG_ARRAY {
      //   MessageContainerView<Chirality::right>& operator*() const {
      //      return *static_cast<MessageContainerView<Chirality::right>*>( &(this->operator*()) ); 
      //   }
      //   MessageContainerView<Chirality::right>& operator[](const INDEX i) const  {
      //      // not supported anymore, just use iterators
      //      assert(false);
      //   }
      //   private:
      //   const MSG_ARRAY& a_;
      //   //MessageContainerView<Chirality::right>& operator[](const INDEX i) const 
      //   //{ 
      //   //   //return *static_cast<MessageContainerView<Chirality::right>*>( (static_cast<const MSG_ARRAY*>(this)->operator[](i)) ); 
      //   //   return *static_cast<MessageContainerView<Chirality::right>*>( (static_cast<const MSG_ARRAY*>(this)->operator[](i)) ); 
      //   //   //return *static_cast<MessageContainerView<Chirality::right>*>( operator[](i) ); 
      //   //   //return operator[](i);
      //   //}
      //};
      //return MessageType::SendMessagesToLeft(rightFactor, repam, static_cast<const ViewWrapper>(msgs.begin()), omegaBegin);
   }

   template<typename LEFT_FACTOR, typename MSG_ARRAY, typename ITERATOR>
   constexpr static bool
   CanCallSendMessagesToRightContainer()
   { 
      return FunctionExistence::HasSendMessagesToRight<MessageType, void, LEFT_FACTOR, MSG_ARRAY, MSG_ARRAY, ITERATOR>(); 
   }
   template<typename LEFT_FACTOR, typename MSG_ARRAY, typename ITERATOR>
   static void SendMessagesToRightContainer(const LEFT_FACTOR& leftFactor, const MSG_ARRAY& msgs, ITERATOR omegaBegin) 
   {
      // do zrobienia: unify message iterators
      struct MessageIterator {
         using type = decltype(msgs.begin());
         MessageIterator(type it) : it_(it) {}
         MessageContainerView<Chirality::left>& operator*() const {
            return *(static_cast<MessageContainerView<Chirality::left>*>( *it_ )); 
         }
         MessageIterator operator++() {
            ++it_;
            return *this;
         }
         bool operator!=(const MessageIterator& o) const {
            return it_ != o.it_;
         }
         private:
         type it_;
      };
      return MessageType::SendMessagesToRight(leftFactor->GetFactor, MessageIterator(msgs.begin()), MessageIterator(msgs.end()), omegaBegin);
      //struct ViewWrapper : public MSG_ARRAY {
      //   MessageContainerView<Chirality::left>& operator[](const INDEX i) const 
      //   { 
      //      assert(false); // make as in SendMessagesToLeftContainer
      //      return *static_cast<MessageContainerView<Chirality::left>*>( (static_cast<const MSG_ARRAY*>(this)->operator[](i)) ); 
      //   }
      //};
      //MessageType::SendMessagesToRight(leftFactor, repam, *static_cast<const ViewWrapper*>(&msgs), omegaBegin);
   }

   constexpr static bool
   CanComputeRightFromLeftPrimal()
   {
      return FunctionExistence::HasComputeRightFromLeftPrimal<MessageType,void,
             PrimalSolutionStorage::Element, decltype(leftFactor_->GetFactor()),
             PrimalSolutionStorage::Element, decltype(rightFactor_->GetFactor())>();
   }
   constexpr static bool
   CanComputeLeftFromRightPrimal()
   {
      return FunctionExistence::HasComputeLeftFromRightPrimal<MessageType,void,
             PrimalSolutionStorage::Element, decltype(leftFactor_->GetFactor()),
             PrimalSolutionStorage::Element, decltype(rightFactor_->GetFactor())>();
   }

   void ComputeRightFromLeftPrimal(typename PrimalSolutionStorage::Element primal) 
   {
      msg_op_.ComputeRightFromLeftPrimal(primal + leftFactor_->GetPrimalOffset(), leftFactor_->GetFactor(), primal + rightFactor_->GetPrimalOffset(), rightFactor_->GetFactor());
      rightFactor_->PropagatePrimal(primal + rightFactor_->GetPrimalOffset());
      rightFactor_->ComputePrimalThroughMessages(primal);
   }

   void ComputeLeftFromRightPrimal(PrimalSolutionStorage::Element primal)
   {
      msg_op_.ComputeLeftFromRightPrimal(primal + leftFactor_->GetPrimalOffset(), leftFactor_->GetFactor(), primal + rightFactor_->GetPrimalOffset(), rightFactor_->GetFactor());
      leftFactor_->PropagatePrimal(primal + leftFactor_->GetPrimalOffset());
      leftFactor_->ComputePrimalThroughMessages(primal);
   }

   constexpr static bool
   CanCheckPrimalConsistency()
   {
      return FunctionExistence::HasCheckPrimalConsistency<MessageType,bool,
          PrimalSolutionStorage::Element, typename LeftFactorContainer::FactorType*,
          PrimalSolutionStorage::Element, typename RightFactorContainer::FactorType*>();
   }

   bool CheckPrimalConsistency(PrimalSolutionStorage::Element primal) const final
   { 
      bool ret;
      static_if<CanCheckPrimalConsistency()>([&](auto f) {
            ret = f(msg_op_).CheckPrimalConsistency(primal + leftFactor_->GetPrimalOffset(), leftFactor_->GetFactor(), primal + rightFactor_->GetPrimalOffset(), rightFactor_->GetFactor());
      }).else_([&](auto f) {
               ret = true;
      });
      return ret;
   }

   // do zrobienia: not needed anymore
   // do zrobienia: this does not capture write back functions not returning REAL&
   constexpr static bool IsAssignableLeft() {
      return FunctionExistence::IsAssignable<LeftFactorType, REAL, INDEX>();
   }
   constexpr static bool IsAssignableRight() {
      return FunctionExistence::IsAssignable<RightFactorType, REAL, INDEX>();
   }

   template<typename ARRAY, bool IsAssignable = IsAssignableLeft()>
   constexpr static bool CanBatchRepamLeft()
   {
      return FunctionExistence::HasRepamLeft<MessageType,void,LeftFactorType,ARRAY>();
   }
   template<typename ARRAY, bool IsAssignable = IsAssignableLeft()>
   //typename std::enable_if<CanBatchRepamLeft<ARRAY>() == true && IsAssignable == true>::type
   void
   RepamLeft(const ARRAY& m)
   { 
      //assert(false); // no -+ distinguishing
      static_if<CanBatchRepamLeft<ARRAY>()>([&](auto f) {
            f(msg_op_).RepamLeft(*(leftFactor_->GetFactor()), m);
      }).else_([&](auto f) {
         for(INDEX i=0; i<m.size(); ++i) {
            f(msg_op_).RepamLeft(*(leftFactor_->GetFactor()), m[i], i);
         }
      });
   }
   /*
   template<typename ARRAY, bool IsAssignable = IsAssignableLeft()>
   typename std::enable_if<CanBatchRepamLeft<ARRAY>() == false && IsAssignable == true>::type
   RepamLeft(const ARRAY& m)
   { 
      //assert(false); // no -+ distinguishing
      for(INDEX i=0; i<m.size(); ++i) {
         msg_op_.RepamLeft(*(leftFactor_->GetFactor()), m[i], i);
      }
   }
   template<typename ARRAY, bool IsAssignable = IsAssignableLeft()>
   typename std::enable_if<IsAssignable == false>::type
   RepamLeft(const ARRAY& m)
   {
   assert(false);
   }
   */

   template<bool IsAssignable = IsAssignableLeft()>
   //typename std::enable_if<IsAssignable == true>::type
   void
   RepamLeft(const REAL diff, const INDEX dim) {
      msg_op_.RepamLeft(*(leftFactor_->GetFactor()), diff, dim); // note: in right, we reparametrize by +diff, here by -diff
   }
   /*
   template<bool IsAssignable = IsAssignableLeft()>
   typename std::enable_if<IsAssignable == false>::type
   RepamLeft(const REAL diff, const INDEX dim)
   {
   assert(false);
   }
   */

   template<typename ARRAY>
   constexpr static bool CanBatchRepamRight()
   {
      // do zrobienia: replace Container by actual factor
      return FunctionExistence::HasRepamRight<MessageType,void,RightFactorType,ARRAY>();
   }
   template<typename ARRAY, bool IsAssignable = IsAssignableRight()>
   //typename std::enable_if<CanBatchRepamRight<ARRAY>() == true && IsAssignable == true>::type
   void
   RepamRight(const ARRAY& m)
   { 
      //assert(false); // no -+ distinguishing
      static_if<CanBatchRepamRight<ARRAY>()>([&](auto f) {
            f(msg_op_).RepamRight(*(rightFactor_->GetFactor()), m);
      }).else_([&](auto f) {
         for(INDEX i=0; i<m.size(); ++i) {
            f(msg_op_).RepamRight(*(rightFactor_->GetFactor()), m[i], i);
         }
      });
   }
   /*
   template<typename ARRAY, bool IsAssignable = IsAssignableRight()>
   typename std::enable_if<CanBatchRepamRight<ARRAY>() == false && IsAssignable == true>::type
   RepamRight(const ARRAY& m)
   {
      //assert(false); // no -+ distinguishing
      for(INDEX i=0; i<m.size(); ++i) {
         msg_op_.RepamRight(*(rightFactor_->GetFactor()), m[i], i);
      }
   }
   template<typename ARRAY, bool IsAssignable = IsAssignableRight()>
   typename std::enable_if<IsAssignable == false>::type
   RepamRight(const ARRAY& m)
   {
   assert(false);
   }
   */

   template<bool IsAssignable = IsAssignableRight()>
   //typename std::enable_if<IsAssignable == true>::type
   void
   RepamRight(const REAL diff, const INDEX dim) {
      msg_op_.RepamRight(*(rightFactor_->GetFactor()), diff, dim);
   }
   /*
   template<bool IsAssignable = IsAssignableRight()>
   typename std::enable_if<IsAssignable == false>::type
   RepamRight(const REAL diff, const INDEX dim)
   {
   assert(false);
   }
   */

   // do zrobienia: better name?
   //REAL GetLeftMessage(const INDEX i) const { return msg_op_.GetLeftMessage(i,*this); }
   //REAL GetRightMessage(const INDEX i) const { return msg_op_.GetRightMessage(i,*this);  }

   //FactorTypeAdapter* GetLeftFactor() const { return leftFactor_; }
   //FactorTypeAdapter* GetRightFactor() const { return rightFactor_; }
   // do zrobienia: Rename Get{Left|Right}FactorContainer
   LeftFactorContainer* GetLeftFactor() const final { return leftFactor_; }
   RightFactorContainer* GetRightFactor() const final { return rightFactor_; }

   //INDEX GetMessageNumber() const final { return MESSAGE_NO; } 
   //REAL GetMessageWeightToRight() const final { return SEND_MESSAGE_TO_RIGHT_WEIGHT::value; }
   //REAL GetMessageWeightToLeft() const final { return SEND_MESSAGE_TO_LEFT_WEIGHT::value;  }
   
   // class for storing a callback upon new assignment of message: update left and right factors
   // convention is as follows: original message is for right factor. Inverted message is for left one
   template<Chirality CHIRALITY>
   class MsgVal {
   public:
      MsgVal(MessageContainerType* msg, const INDEX dim) : 
         msg_(msg), 
         dim_(dim)
      {}
      // do zrobienia: do not support this operation! Goal is to not hold messages anymore, except for implicitly held reparametrizations.
      /*
      MsgVal& operator=(const REAL x) __attribute__ ((always_inline))
      {
         assert(false);
         const REAL diff = x - msg_->operator[](dim_);
         // set new message
         static_cast<typename MessageContainerType::MessageStorageType*>(msg_)->operator[](dim_) = x;
         // propagate difference to left and right factor
         msg_->RepamLeft( diff, dim_);
         msg_->RepamRight( diff, dim_);
         return *this;
      }
      */
      MsgVal& operator-=(const REAL x) __attribute__ ((always_inline))
      {
         if(CHIRALITY == Chirality::right) { // message is computed by right factor
            msg_->RepamLeft( +x, dim_);
            msg_->RepamRight(-x, dim_);
         } else if (CHIRALITY == Chirality::left) { // message is computed by left factor
            msg_->RepamLeft(  -x, dim_);
            msg_->RepamRight( +x, dim_);
            //msg_->RepamLeft( +x, dim_);
            //msg_->RepamRight( +x, dim_);
         } else {
            assert(false);
         }
         return *this;
      }
      MsgVal& operator+=(const REAL x) __attribute__ ((always_inline))
      {
         assert(false);
         if(CHIRALITY == Chirality::right) { // message is computed by right factor
            msg_->RepamLeft( x, dim_);
            msg_->RepamRight( x, dim_);
         } else if(CHIRALITY == Chirality::left) { // message is computed by left factor
            msg_->RepamLeft( x, dim_);
            msg_->RepamRight( x, dim_);
            //msg_->RepamLeft( -x, dim_);
            //msg_->RepamRight( -x, dim_);
         } else {
            assert(false);
         }
         return *this;
      }
      // do zrobienia: this value should never be used. Remove function
      //operator REAL() const __attribute__ ((always_inline)) { return static_cast<typename MessageContainerType::MessageStorageType*>(msg_)->operator[](dim_); }
   private:
      MessageContainerType* const msg_;
      const INDEX dim_;
   };

   // this view of the message container is given to left and right factor respectively when receiving or sending messages
   template<Chirality CHIRALITY>
   class MessageContainerView : public MessageContainerType {
   public:
      //using MessageContainerType;
      MsgVal<CHIRALITY> operator[](const INDEX i) 
      {
         return MsgVal<CHIRALITY>(this,i);
      }

      template<typename ARRAY>
      MessageContainerType& operator-=(const ARRAY& diff) {
        MinusVec<ARRAY> minus_diff(diff);
        // note: order of below operations is important: When the message is e.g. just the potential, we must reparametrize the other side first!
        if(CHIRALITY == Chirality::right) {
          RepamLeft(diff);
          RepamRight(-diff);
        } else if(CHIRALITY == Chirality::left) {
          RepamRight(diff);
          RepamLeft(-diff);
        } else {
          assert(false);
        }
        return *this;
      }

   };

   // for primal computation: record message change only in one side and into a special array
   template<Chirality CHIRALITY>
   class OneSideMsgVal
   {
   public:
      OneSideMsgVal(MessageContainerType* msg, const INDEX dim) : 
         msg_(msg), 
         dim_(dim)
      {}

      OneSideMsgVal& operator-=(const REAL x) __attribute__ ((always_inline))
      {
         if(CHIRALITY == Chirality::right) { // message is computed by right factor
            msg_->RepamRight(+x, dim_);
         } else if (CHIRALITY == Chirality::left) { // message is computed by left factor
            msg_->RepamLeft(+x, dim_);
         } else {
            assert(false);
         }
         return *this;
      }

      OneSideMsgVal& operator+=(const REAL x) __attribute__ ((always_inline))
      {
         assert(false);
         if(CHIRALITY == Chirality::right) {
            msg_->RepamRight(+x, dim_);
         } else if(CHIRALITY == Chirality::left) {
            msg_->RepamLeft(+x, dim_);
         } else {
            assert(false);
         }
         return *this;
      }

   private:
      MessageContainerType* const msg_;
      const INDEX dim_;
   };

   // this view is given to receive restricted message operations. 
   // Reparametrization is recorded only on one side
   template<Chirality CHIRALITY>
   class OneSideMessageContainerView : public MessageContainerType{
   public:
      //using MessageContainerType;
      OneSideMsgVal<CHIRALITY> operator[](const INDEX i) 
      {
         return OneSideMsgVal<CHIRALITY>(this,i);
      }

      template<typename ARRAY>
      MessageContainerType& operator-=(const ARRAY& diff) {
        MinusVec<ARRAY> minus_diff(diff);
        if(CHIRALITY == Chirality::right) {
          RepamRight(-diff);
        } else if(CHIRALITY == Chirality::left) {
          RepamLeft(-diff);
        } else {
          assert(false);
        }
        return *this;
      }

   };


   // there must be four different implementations of msg updating with SIMD: 
   // (i) If parallel reparametrization is not supported by either left and right factor
   // If (ii) left or (iii) right factor supports reparametrization but not both
   // If (iv) both left and right factor support reparametrization


   template<typename ARRAY>
   MessageContainerType& operator-=(const ARRAY& diff) {
      assert(false); // update to left right -+
      MinusVec<ARRAY> minus_diff(diff);
      assert(minus_diff.size() == this->size());
      RepamLeft(-diff);
      RepamRight(-diff);
      return *this;
   }

   template<typename ARRAY>
   MessageContainerType& operator+=(const ARRAY& diff) {
      assert(false); // update to left right -+
      PlusVec<ARRAY> plus_diff(diff); // used to wrap Vc::Memory, otherwise not needed // do zrobienia: change this with better vector architecture
      assert(plus_diff.size() == this->size()); // or entriesCount
      RepamLeft(plus_diff);
      RepamRight(plus_diff);
      return *this;
   }

   // possibly not the best choice: Sometimes msg_op_ needs access to this class
   const MessageType& GetMessageOp() const
   {
      return msg_op_;
   }

   constexpr static bool CanCreateConstraints()
   {
      //return FunctionExistence::HasCreateConstraints<MessageType,LpInterfaceAdapter*, LeftFactorContainer*, RightFactorContainer*>();
      return FunctionExistence::HasCreateConstraints<MessageType,void, LpInterfaceAdapter*, LeftFactorType*, RightFactorType*>();
   }
   
   virtual void CreateConstraints(LpInterfaceAdapter* l) final
   {
      // do zrobienia: use static_if
      static_if<CanCreateConstraints()>([&](auto f) {
            f(msg_op_).CreateConstraints(l,leftFactor_->GetFactor(),rightFactor_->GetFactor());
      }).else_([&](auto f) {
         throw std::runtime_error("create constraints not implemented by message");
      });
   }


protected:
   MessageType msg_op_; // possibly inherit privately from MessageType to apply empty base optimization when applicable
   LeftFactorContainer* const leftFactor_;
   RightFactorContainer* const rightFactor_;

   // see notes on allocator in FactorContainer
   struct Allocator {
      using type = MemoryPool<MessageContainerType,4096*sizeof(MessageContainerType)>; 
      static type& get() {
         static type allocator;
         return allocator;
      }
   };
};


// container class for factors. Here we hold the factor, all connected messages, reparametrization storage and perform reparametrization and coordination for sending and receiving messages.
// derives from REPAM_STORAGE_TYPE to mixin a class for storing the reparametrized potential
// implements the interface from FactorTypeAdapter for access from LP_MP
// if COMPUTE_PRIMAL_SOLUTION is true, MaximizePotential is expected to return either an integer of type INDEX or a std::vector<INDEX>
// if WRITE_PRIMAL_SOLUTION is false, WritePrimal will not output anything
// do zrobienia: introduce enum classes for COMPUTE_PRIMAL_SOLUTION and WRITE_PRIMAL_SOLUTION
template<typename FACTOR_TYPE, 
         //template<class> class REPAM_STORAGE_TYPE, 
         class FACTOR_MESSAGE_TRAIT,
         INDEX FACTOR_NO,
         bool COMPUTE_PRIMAL_SOLUTION = false> 
class FactorContainer : public FactorTypeAdapter
{
public:
   using FactorContainerType = FactorContainer<FACTOR_TYPE, FACTOR_MESSAGE_TRAIT, FACTOR_NO, COMPUTE_PRIMAL_SOLUTION>;
   using FactorType = FACTOR_TYPE;

   // do zrobienia: templatize cosntructor to allow for more general initialization of reparametrization storage and factor
   template<typename ...ARGS>
   FactorContainer(ARGS... args) : factor_(args...) {}

   FactorContainer(const FactorType&& factor) : factor_(std::move(factor)) {
      //INDEX status;
      //std::cout << "msg_ type= "  << abi::__cxa_demangle(typeid(msg_).name(),0,0,&status) << "\n";
      //std::cout << "dispatcher list = "  << abi::__cxa_demangle(typeid(MESSAGE_DISPATCHER_TYPELIST).name(),0,0,&status) << "\n";
      //std::cout << "msg_ type= "  << abi::__cxa_demangle(typeid(msg_).name(),0,0,&status) << "\n";
      //std::cout << "left message list = " << abi::__cxa_demangle(typeid(left_message_list_).name(),0,0,&status) << "\n";
      //std::cout << "left message list = " << abi::__cxa_demangle(typeid(left_message_list_1).name(),0,0,&status) << "\n";
   
   }
   FactorContainer(const FactorType& factor) : factor_(factor) 
   {}
   ~FactorContainer() { 
      static_assert(meta::unique<MESSAGE_DISPATCHER_TYPELIST>::size() == MESSAGE_DISPATCHER_TYPELIST::size(), 
            "Message dispatcher typelist must have unique elements");
      static_assert(FACTOR_NO >= 0 && FACTOR_NO < FACTOR_MESSAGE_TRAIT::FactorList::size(), "factor number must be smaller than length of factor list");
   }

   // overloaded new so that factor containers are allocated by global block allocator consecutively
   void* operator new(std::size_t size)
   {
      assert(size == sizeof(FactorContainerType));
      //return (void*) global_real_block_allocator.allocate(size/sizeof(REAL),1);
      return Allocator::get().allocate(1);
   }
   void operator delete(void* mem)
   {
      Allocator::get().deallocate((FactorContainerType*) mem);
      //assert(false);
      //global_real_block_allocator.deallocate(mem,sizeof(FactorContainerType));
   }


   template<typename MESSAGE_DISPATCHER_TYPE, typename MESSAGE_TYPE> 
   void AddMessage(MESSAGE_TYPE* m) { 
      constexpr INDEX n = FindMessageDispatcherTypeIndex<MESSAGE_DISPATCHER_TYPE>();
      static_assert( n < meta::size<MESSAGE_DISPATCHER_TYPELIST>(), "message dispatcher not supported");
      static_assert( n < std::tuple_size<decltype(msg_)>(), "message dispatcher not supported");
      //INDEX status;
      //std::cout << "msg dispatcher list =\n" << abi::__cxa_demangle(typeid(MESSAGE_DISPATCHER_TYPELIST).name(),0,0,&status) << "\n";
      //std::cout << "dispatcher  type =\n" << abi::__cxa_demangle(typeid(MESSAGE_DISPATCHER_TYPE).name(),0,0,&status) << "\n";
      //std::cout << " number = " << n << "\n" ;
      //std::cout << "message type = " << abi::__cxa_demangle(typeid(MESSAGE_TYPE).name(),0,0,&status) << "\n";

      std::get<n>(msg_).push_back(m);
   }

   // do zrobienia: remove
   // get sum of all messages in dimension i -> obsolete
   const REAL GetMessageSum(const INDEX i) const {
      return GetMessageSum(MESSAGE_DISPATCHER_TYPELIST{},i);
   }
   template<typename... MESSAGE_DISPATCHER_TYPES_REST>
   const REAL GetMessageSum(meta::list<MESSAGE_DISPATCHER_TYPES_REST...>, const INDEX i) const { return 0.0; }
   template<typename MESSAGE_DISPATCHER_TYPE, typename... MESSAGE_DISPATCHER_TYPES_REST>
   const REAL GetMessageSum(meta::list<MESSAGE_DISPATCHER_TYPE, MESSAGE_DISPATCHER_TYPES_REST...>, const INDEX i) const {
      //INDEX status;
      //std::cout << "return message for " << abi::__cxa_demangle(typeid(MESSAGE_DISPATCHER_TYPE).name(),0,0,&status) << "\n";
      // current number of MESSAGE_DISPATCHER_TYPE
      constexpr INDEX n = FindMessageDispatcherTypeIndex<MESSAGE_DISPATCHER_TYPE>();
      REAL msg_val = 0.0;
      for(auto it=std::get<n>(msg_).begin(); it!=std::get<n>(msg_).end(); ++it) {
         msg_val += MESSAGE_DISPATCHER_TYPE::GetMessage(*(*it),i);
      }
      // receive messages for subsequent MESSAGE_DISPATCER_TYPES
      return msg_val + GetMessageSum(meta::list<MESSAGE_DISPATCHER_TYPES_REST...>{},i);
   }

   void UpdateFactor(const std::vector<REAL>& omega) final
   {
      ReceiveMessages(omega);
      MaximizePotential();
      SendMessages(omega);
   }

   // do zrobienia: possibly also check if method present
   constexpr static bool
   CanComputePrimal()
   {
      return COMPUTE_PRIMAL_SOLUTION;
   }

   constexpr static bool
   CanPropagatePrimal()
   {
      return FunctionExistence::HasPropagatePrimal<FactorType,void,PrimalSolutionStorage::Element>();
   }

   void PropagatePrimal(PrimalSolutionStorage::Element primal) 
   {
      static_if<CanPropagatePrimal()>([&](auto f) {
            f(factor_).PropagatePrimal(primal);
      });
   }

   constexpr static bool
   CanMaximizePotential()
   {
      return FunctionExistence::HasMaximizePotential<FactorType,void>();
   }

   void UpdateFactor(const std::vector<REAL>& omega, typename PrimalSolutionStorage::Element primal) final
   {
      if(CanComputePrimal()) { // do zrobienia: for now
         if(CanReceiveRestrictedMessages()) {
           //std::cout << "before tmp repam allocation\n";
           // do zrobienia: copy whole factor here
            auto cur_factor(factor_);
            //using vector_type = std::vector<REAL, stack_allocator<REAL>>;
            //vector_type tmpRepam(this->size(), 0.0, global_real_stack_allocator);
            //std::vector<REAL> tmpRepam(this->size()); // temporary structure where repam is stored before it is reverted back.
            //for(INDEX i=0; i<tmpRepam.size(); ++i) {
            //   tmpRepam[i] = factor_[i];
            //}
            // first we compute restricted incoming messages, on which to compute the primal
            ReceiveRestrictedMessages(primal);
            // now we compute primal
            MaximizePotentialAndComputePrimal(primal);
            // restore original reparametrization
            factor_ = cur_factor;
            //for(INDEX i=0; i<tmpRepam.size(); ++i) {
            //   factor_[i] = tmpRepam[i];
            //}
            MaximizePotential();
           //std::cout << "before tmp repam deallocation\n";
         } else {
            MaximizePotentialAndComputePrimal(primal);
         }
         // now prapagate primal to adjacent factors
         ComputePrimalThroughMessages(primal);
      } else {
         MaximizePotential();
      } 

      ReceiveMessages(omega);
      MaximizePotential();
      SendMessages(omega);
   }

   void MaximizePotential()
   {
      static_if<CanMaximizePotential()>([&](auto f) {
            f(factor_).MaximizePotential();
      });
   }

   void MaximizePotentialAndComputePrimal(typename PrimalSolutionStorage::Element primal)
   {
      static_if<COMPUTE_PRIMAL_SOLUTION>([&](auto f) {
            //f(factor_).MaximizePotentialAndComputePrimal(*this, primal + primalOffset_);
            f(factor_).MaximizePotentialAndComputePrimal(primal + primalOffset_);
      });
   }

   // do zrobienia: rename PropagatePrimalThroughMessages
   void ComputePrimalThroughMessages(typename PrimalSolutionStorage::Element primal) const
   {
      meta::for_each(MESSAGE_DISPATCHER_TYPELIST{}, [this,primal](auto l) {
            static_if<l.CanComputePrimalThroughMessage()>([&](auto f) {
                  constexpr INDEX n = FindMessageDispatcherTypeIndex<decltype(l)>();
                  for(auto it = std::get<n>(msg_).begin(); it != std::get<n>(msg_).end(); ++it) {
                     f(l.ComputePrimalThroughMessage)(*(*it), primal);
                  }
            });
      });
   }

   void ReceiveMessages(const std::vector<REAL>& omega) 
   {
      // note: currently all messages are received, even if not needed. Change this again.
      auto omegaIt = omega.begin();
      meta::for_each(MESSAGE_DISPATCHER_TYPELIST{}, [this,&omegaIt](auto l) {
            constexpr INDEX n = FindMessageDispatcherTypeIndex<decltype(l)>();
            static_if<l.CanCallReceiveMessage()>([&](auto f) {
                  
                  for(auto it = std::get<n>(msg_).begin(); it != std::get<n>(msg_).end(); ++it) {
                     //if(*omegaIt == 0.0) { // makes large difference for cosegmentation_bins, why?
                     f(l.ReceiveMessage)(*(*it));
                     //}
                  }

                  });
            
            std::advance(omegaIt, std::get<n>(msg_).size());
      });
   }

   // we write message change not into original reparametrization, but into temporary one named pot
   void ReceiveRestrictedMessages(PrimalSolutionStorage::Element primal) 
   {
      meta::for_each(MESSAGE_DISPATCHER_TYPELIST{}, [this,primal](auto l) {
            constexpr INDEX n = FindMessageDispatcherTypeIndex<decltype(l)>();
            static_if<l.CanCallReceiveRestrictedMessage()>([&](auto f) {
                  for(auto it=std::get<n>(msg_).begin(); it != std::get<n>(msg_).end(); ++it) {
                     f(l.ReceiveRestrictedMessage)(*(*it),primal); // do zrobienia: only receive messages from sensible ones
                  }
            });
      });
   }

   constexpr static bool CanReceiveRestrictedMessages() 
   {
      struct can_receive {
         template<typename MESSAGE_DISPATCHER_TYPE>
            using invoke = typename std::is_same<std::integral_constant<bool,MESSAGE_DISPATCHER_TYPE::CanCallReceiveMessage()>, std::integral_constant<bool,true> >::type;
      };
      return meta::any_of<MESSAGE_DISPATCHER_TYPELIST, can_receive>{};
   }

   // methods used by MessageIterator
   const INDEX GetNoMessages() const final
   {
      INDEX noMessages = 0;
      meta::for_each(MESSAGE_DISPATCHER_TYPELIST{}, [this,&noMessages](auto l) {
            constexpr INDEX n = FindMessageDispatcherTypeIndex<decltype(l)>();
            noMessages += std::get<n>(msg_).size();
            } );
      return noMessages;
   }

   INDEX no_send_messages_calls() const {
      INDEX no_calls = 0;
      meta::for_each(MESSAGE_DISPATCHER_TYPELIST{}, [this,&no_calls](auto l) {
            constexpr INDEX n = FindMessageDispatcherTypeIndex<decltype(l)>();
            if(CanCallSendMessages(l)) {
               if(std::get<n>(msg_).size() > 0) {
                  ++no_calls;
               }
            } else if(CanCallSendMessage(l)) {
               no_calls += std::get<n>(msg_).size();
            }
            } );
      return no_calls;
   }

   void SendMessages(const std::vector<REAL>& omega) 
   {
      //assert(omega.size() == GetNoMessages()); // this is not true: omega.size() is the number of messages that implement a send function
      const INDEX no_calls = no_send_messages_calls();

      // do zrobienia: also do not construct currentRepam, if exactly one message update call will be issued. 
      // Check if there is one message dispatcher such that its size can be called via a constexpr function and is 1 -> complicated!
      // also possible: check whether omega has only one nonnegative entry
      if( no_calls > 0 ) { // no need to construct currentRepam, if it will not be used at all
         // make a copy of the current reparametrization. The new messages are computed on it. Messages are updated implicitly and hence possibly the new reparametrization is automatically adjusted, which would interfere with message updates
         
         //std::vector<REAL> repam_delta(RepamStorageType::size(),0.0); // here we store the change in the reparametrization produced by SnedMessage. Alternatively we could store the messages that are produced. Check which is less overhead and choose it so.

         FactorType tmp_factor = factor_; 

         auto omegaIt = omega.begin();

         meta::for_each(MESSAGE_DISPATCHER_TYPELIST{}, [&](auto l) {
               constexpr INDEX n = FindMessageDispatcherTypeIndex<decltype(l)>();
               // check whether the message supports batch updates. If so, call batch update. If not, check whether individual updates are supported. If yes, call individual updates. If no, do nothing
               static_if<CanCallSendMessages(l)>([&](auto f) {
                     const REAL omega_sum = std::accumulate(omegaIt, omegaIt + std::get<n>(msg_).size(), 0.0);
                     if(omega_sum > 0.0) { 
                     assert(false); // must give copy of factor
                        l.SendMessages(tmp_factor, std::get<n>(msg_), omegaIt);
                     }
                     omegaIt += std::get<n>(msg_).size();
               }).else_([&](auto f) {
                  static_if<CanCallSendMessage(l)>([&](auto f) {
                        for(auto it = std::get<n>(msg_).begin(); it != std::get<n>(msg_).end(); ++it, ++omegaIt) {
                           if(*omegaIt != 0.0) {
                              l.SendMessage(&tmp_factor, *(*it), *omegaIt); 
                           }
                        }
                  });
               });
         });
         assert(omegaIt == omega.end());
      }
   }



   template<typename ...MESSAGE_DISPATCHER_TYPES_REST>
   MessageTypeAdapter* GetMessage(meta::list<MESSAGE_DISPATCHER_TYPES_REST...> t, const INDEX msgNo) const 
   {
      assert(false);
      throw std::runtime_error("index out of bound");
      return nullptr;
   }
   template<typename MESSAGE_DISPATCHER_TYPE, typename ...MESSAGE_DISPATCHER_TYPES_REST>
   MessageTypeAdapter* GetMessage(meta::list<MESSAGE_DISPATCHER_TYPE, MESSAGE_DISPATCHER_TYPES_REST...> t, const INDEX msgNo) const 
   {
      constexpr INDEX n = FindMessageDispatcherTypeIndex<MESSAGE_DISPATCHER_TYPE>();
      if(msgNo < std::get<n>(msg_).size()) {
         auto it = std::get<n>(msg_).begin();
         for(INDEX i=0; i<msgNo; ++i) { ++it; }
         return *it;
         //return  std::get<n>(msg_)[msgNo]; 
      }
      else return GetMessage(meta::list<MESSAGE_DISPATCHER_TYPES_REST...>{}, msgNo - std::get<n>(msg_).size());
   }
   MessageTypeAdapter* GetMessage(const INDEX n) const final
   {
      assert(n<GetNoMessages());
      return GetMessage(MESSAGE_DISPATCHER_TYPELIST{}, n);
   }

   template<typename ...MESSAGE_DISPATCHER_TYPES_REST>
   FactorTypeAdapter* GetConnectedFactor(meta::list<MESSAGE_DISPATCHER_TYPES_REST...> t, const INDEX cur_msg_idx) const 
   {
      throw std::runtime_error("message index out of bound");
   }
   template<typename MESSAGE_DISPATCHER_TYPE, typename ...MESSAGE_DISPATCHER_TYPES_REST>
   FactorTypeAdapter* GetConnectedFactor(meta::list<MESSAGE_DISPATCHER_TYPE, MESSAGE_DISPATCHER_TYPES_REST...> t, const INDEX cur_msg_idx) const // to get the current message_type
   {
      constexpr INDEX n = FindMessageDispatcherTypeIndex<MESSAGE_DISPATCHER_TYPE>();
      const INDEX no_msgs = std::get<n>(msg_).size();
      if(cur_msg_idx < no_msgs) {
         //auto msg = std::get<n>(msg_)[cur_msg_idx];
         // do zrobienia: not most efficient way
         auto it = std::get<n>(msg_).begin();
         for(INDEX i=0; i<cur_msg_idx; ++i) { ++it; }
         auto msg = *it;
         assert(msg != nullptr);
         if(msg->GetLeftFactor() == static_cast<const FactorTypeAdapter*>(this)) { return msg->GetRightFactor(); }
         else { return msg->GetLeftFactor(); }
      } else {
         return GetConnectedFactor(meta::list<MESSAGE_DISPATCHER_TYPES_REST...>{}, cur_msg_idx - no_msgs);
      }
   }
   FactorTypeAdapter* GetConnectedFactor (const INDEX msg_idx) const final
   { 
      auto f = GetConnectedFactor(MESSAGE_DISPATCHER_TYPELIST{}, msg_idx);
      assert(f != this);
      return f;
   }

   constexpr static bool CanReceiveMessages() 
   {
      struct can_receive_message {
         template<typename MESSAGE_DISPATCHER_TYPE>
            using invoke = typename std::is_same<std::integral_constant<bool,MESSAGE_DISPATCHER_TYPE::CanCallReceiveMessage()>, std::integral_constant<bool,true> >::type;
      };
      return meta::any_of<MESSAGE_DISPATCHER_TYPELIST, can_receive_message>{};
   }

   bool CallsReceiveMessages() const
   {
      bool can_receive = false;
      meta::for_each(MESSAGE_DISPATCHER_TYPELIST{}, [this,&can_receive](auto l) {
         constexpr INDEX n = FindMessageDispatcherTypeIndex<decltype(l)>();
         if(l.CanCallReceiveMessage() && std::get<n>(msg_).size() > 0) {
            can_receive = true;
         }
      });
      return can_receive;
   }


   template<typename MESSAGE_DISPATCHER_TYPE>
   constexpr static bool CanCallSendMessages(MESSAGE_DISPATCHER_TYPE t) 
   {
      constexpr INDEX n = FindMessageDispatcherTypeIndex<MESSAGE_DISPATCHER_TYPE>();
      return MESSAGE_DISPATCHER_TYPE::template CanCallSendMessages<FactorContainerType, decltype(std::get<n>(msg_)), std::vector<REAL>::iterator>();
   }
   template<typename MESSAGE_DISPATCHER_TYPE>
   constexpr static bool CanCallSendMessage(MESSAGE_DISPATCHER_TYPE t) 
   {
      constexpr INDEX n = FindMessageDispatcherTypeIndex<MESSAGE_DISPATCHER_TYPE>();
      return MESSAGE_DISPATCHER_TYPE::CanCallSendMessage();
   }

   constexpr static bool CanSendMessages() 
   {
      struct can_send_message {
         template<typename MESSAGE_DISPATCHER_TYPE>
            using invoke = typename std::is_same<std::integral_constant<bool,CanCallSendMessage(MESSAGE_DISPATCHER_TYPE{})>, std::integral_constant<bool,true> >::type;
      };
      struct can_send_messages {
         template<typename MESSAGE_DISPATCHER_TYPE>
            using invoke = typename std::is_same<std::integral_constant<bool,CanCallSendMessages(MESSAGE_DISPATCHER_TYPE{})>, std::integral_constant<bool,true> >::type;
      };
      return meta::any_of<MESSAGE_DISPATCHER_TYPELIST, can_send_message>{} || meta::any_of<MESSAGE_DISPATCHER_TYPELIST, can_send_messages>{};
   }

   // check whether actually send messages is called. Can be false, even if CanSendMessages is true, e.g. when no message is present
   bool CallsSendMessages() const
   {
      bool calls_send = false;
      meta::for_each(MESSAGE_DISPATCHER_TYPELIST{}, [&](auto l) {
            constexpr INDEX n = FindMessageDispatcherTypeIndex<decltype(l)>();
            if(CanCallSendMessage(l) || CanCallSendMessages(l)) {
               if(std::get<n>(msg_).size()>0) {
                  calls_send = true;
               }
            }
      });
      return calls_send;
   }


   template<typename ...MESSAGE_DISPATCHER_TYPES_REST>
   bool CanSendMessage(meta::list<MESSAGE_DISPATCHER_TYPES_REST...> t, const INDEX cur_msg_idx) const 
   {
      throw std::runtime_error("message index out of bound");
   }
   template<typename MESSAGE_DISPATCHER_TYPE, typename ...MESSAGE_DISPATCHER_TYPES_REST>
   bool CanSendMessage(meta::list<MESSAGE_DISPATCHER_TYPE, MESSAGE_DISPATCHER_TYPES_REST...> t, const INDEX cur_msg_idx) const // to get the current MESSAGE_TYPE
   {
      constexpr INDEX n = FindMessageDispatcherTypeIndex<MESSAGE_DISPATCHER_TYPE>();
      const INDEX no_msgs = std::get<n>(msg_).size();
      if(cur_msg_idx < no_msgs) {
         if( MESSAGE_DISPATCHER_TYPE::CanCallSendMessage() || 
             MESSAGE_DISPATCHER_TYPE::template CanCallSendMessages<decltype(*this), decltype(std::get<n>(msg_)), std::vector<REAL>::iterator>() )
            return true;
         else return false;
      } else {
         return CanSendMessage(meta::list<MESSAGE_DISPATCHER_TYPES_REST...>{}, cur_msg_idx - no_msgs);
      }
   }

   bool CanSendMessage(const INDEX msg_idx) const final
   {
      return CanSendMessage(MESSAGE_DISPATCHER_TYPELIST{}, msg_idx);
   }

   // check whether actually receive restricted messages is called. Can be false, even if CanReceiveRestrictedMessages is true, e.g. when no message is present
   bool CallsReceiveRestrictedMessages() const
   {
      bool calls_receive_restricted = false;
      meta::for_each(MESSAGE_DISPATCHER_TYPELIST{}, [&](auto l) {
            constexpr INDEX n = FindMessageDispatcherTypeIndex<decltype(l)>();
            if(l.CanCallReceiveRestrictedMessage() && std::get<n>(msg_).size() > 0) {
               calls_receive_restricted = true;
            }
      });
      return calls_receive_restricted;
   }

   // does factor call {Receive(Restricted)?|Send}Messages or does it compute primal? If not, UpdateFactor need not be called.
   bool FactorUpdated() const final
   {
      if(CanComputePrimal()) {
         return true;
      }
      if(CanReceiveMessages() && CallsReceiveMessages()) {
         return true;
      }
      if(CanSendMessages() && CallsSendMessages()) {
         return true;
      }
      if(CanReceiveRestrictedMessages() && CallsReceiveRestrictedMessages()) {
         return true;
      }
      return false;
   }

   template<typename ITERATOR>
   void SetAndPropagatePrimal(PrimalSolutionStorage::Element primal, ITERATOR label) const
   {
     //assert(GetPrimalOffset() + PrimalSize() <= primal.size());
      for(INDEX i=0; i<PrimalSize(); ++i) {
         primal[i + GetPrimalOffset()] = label[i];
      }
      ComputePrimalThroughMessages(primal);
   }

   // do zrobienia: possibly do it with std::result_of
   //auto begin() -> decltype(std::declval<RepamStorageType>().begin()) { return RepamStorageType::begin(); }
   //auto end()   -> decltype(std::declval<RepamStorageType>().end()) { return RepamStorageType::end(); }
   //auto cbegin() -> decltype(std::declval<RepamStorageType>().cbegin()) const { return RepamStorageType::cbegin(); } // do zrobienia: somehow discards const qualifiers
   //auto cend()   -> decltype(std::declval<RepamStorageType>().cend()) const { return RepamStorageType::cend(); } 

   /*
   const INDEX size() const { return factor_.size(); }
   // get reparametrized cost
   const REAL operator[](const INDEX i) const { return RepamStorageType::operator[](i); }
   //auto operator[](const INDEX i) -> decltype(std::declval<RepamStorageType>().operator[](0)) { return RepamStorageType::operator[](i); }
   REAL& operator[](const INDEX i) { return RepamStorageType::operator[](i); }
   */

   std::vector<REAL> GetReparametrizedPotential() const final
   {
      assert(false);
      std::vector<REAL> repam(size());
      //for(INDEX i=0; i<repam.size(); ++i) {
      //   repam[i] = RepamStorageType::operator[](i);
      //}
      return repam;
   }

   INDEX size() const final { 
      return factor_.size();
      //return RepamStorageType::size(); 
   }

   constexpr static bool CanComputePrimalSize()
   {
      return FunctionExistence::HasPrimalSize<FactorType,INDEX>();
   }
   template<bool ENABLE = CanComputePrimalSize()>
   typename std::enable_if<!ENABLE,INDEX>::type
   PrimalSizeImpl() const
   {
      return this->size();
   }
   template<bool ENABLE = CanComputePrimalSize()>
   typename std::enable_if<ENABLE,INDEX>::type
   PrimalSizeImpl() const
   {
      return factor_.PrimalSize();
   }
   // return size for primal storage
   INDEX PrimalSize() const final { return PrimalSizeImpl(); }

   REAL LowerBound() const final {
      //return factor_.LowerBound(*this); 
      return factor_.LowerBound(); 
   } 

   FactorType* GetFactor() const { return &factor_; }
   FactorType* GetFactor() { return &factor_; }
   void SetPrimalOffset(const INDEX n) final { primalOffset_ = n; } // this function is used in AddFactor in LP class
   INDEX GetPrimalOffset() const final { return primalOffset_; }

  void SetAuxOffset(const INDEX n) final { auxOffset_ = n; }
  INDEX GetAuxOffset() const final { return auxOffset_; }
   
protected:
   FactorType factor_; // the factor operation
   INDEX primalOffset_;
   INDEX auxOffset_; // do zrobienia: remove again: artifact from LP interface

   // pool memory allocator specific for this factor container
   // note: the below construction is not perfect when more than one solver is run simultaneously: The same allocator is used, yet the optimization problems are different + not thread safe.
   // -> investigate thread_local inline static! inline static however is only supported in C++17
   struct Allocator { // we enclose static allocator in nested class as only there (since C++11) we can access sizeof(FactorContainerType).
      using type = MemoryPool<FactorContainerType,4096*sizeof(FactorContainerType)>; 
      static type& get() {
         static type allocator;
         return allocator;
      }
   };
   
   // compile time metaprogramming to transform Factor-Message information into lists of which messages this factor must hold
   // first get lists with left and right message types
   struct get_msg_type_list {
      template<typename LIST>
         using invoke = typename LIST::MessageContainerType;
   };
   struct get_left_msg {
      template<class LIST>
         using invoke = typename std::is_same<meta::size_t<LIST::leftFactorNumber>, meta::size_t<FACTOR_NO>>::type;
   };
   struct get_right_msg {
      template<class LIST>
         using invoke = typename std::is_same<meta::size_t<LIST::rightFactorNumber>, meta::size_t<FACTOR_NO> >::type;
   };
   struct get_left_msg_container_type_list {
      template<class LIST>
         using invoke = typename LIST::LeftMessageContainerStorageType;
   };
   struct get_right_msg_container_type_list {
      template<class LIST>
         using invoke = typename LIST::RightMessageContainerStorageType;
   };

   using left_msg_list = meta::transform< meta::filter<typename FACTOR_MESSAGE_TRAIT::MessageList, get_left_msg>, get_msg_type_list>;
   using right_msg_list = meta::transform< meta::filter<typename FACTOR_MESSAGE_TRAIT::MessageList, get_right_msg>, get_msg_type_list>;
   using left_msg_container_list = meta::transform< meta::filter<typename FACTOR_MESSAGE_TRAIT::MessageList, get_left_msg>, get_left_msg_container_type_list>;
   using right_msg_container_list = meta::transform< meta::filter<typename FACTOR_MESSAGE_TRAIT::MessageList, get_right_msg>, get_right_msg_container_type_list>;

   // now construct a tuple with left and right dispatcher
   struct left_dispatch {
      template<class LIST>
         using invoke = MessageDispatcher<LIST, LeftMessageFuncGetter>;
   };
   struct right_dispatch {
      template<class LIST>
         using invoke = MessageDispatcher<LIST, RightMessageFuncGetter>;
   };
   using left_dispatcher_list = meta::transform< left_msg_list, left_dispatch >;
   using right_dispatcher_list = meta::transform< right_msg_list, right_dispatch >;

   using MESSAGE_DISPATCHER_TYPELIST = meta::concat<left_dispatcher_list, right_dispatcher_list>;

   // helper function for getting the index in msg_ of given MESSAGE_DISPATCHER_TYPE
   template<typename MESSAGE_DISPATCHER_TYPE>
   static constexpr INDEX FindMessageDispatcherTypeIndex()
   {
      constexpr INDEX n = meta::find_index<MESSAGE_DISPATCHER_TYPELIST, MESSAGE_DISPATCHER_TYPE>::value;
      static_assert(n < meta::size<MESSAGE_DISPATCHER_TYPELIST>::value,"");
      return n;
   }


   // construct tuple holding messages for left and right dispatch
   // the tuple will hold some container for the message type. The container type is specified in the {Left|Right}MessageContainerStorageType fields of MessageList
   using msg_container_type_list = meta::concat<left_msg_container_list, right_msg_container_list>;

   tuple_from_list<msg_container_type_list> msg_;

public:
   REAL EvaluatePrimal(typename PrimalSolutionStorage::Element primalIt) const final
   {
      //return factor_.EvaluatePrimal(*this,primalIt + primalOffset_);
      return factor_.EvaluatePrimal(primalIt + primalOffset_);
   }

   constexpr static bool CanCreateConstraints()
   {
      return FunctionExistence::HasCreateConstraints<FactorType,void,LpInterfaceAdapter*>();
   }

   constexpr static bool CanReduceLp()
   {
      return FunctionExistence::HasReduceLp<FactorType,void,LpInterfaceAdapter*, FactorContainerType&>();
   }
   template<bool ENABLE = CanReduceLp()>
   typename std::enable_if<!ENABLE>::type
   ReduceLpImpl(LpInterfaceAdapter* l) const
   {}  
   template<bool ENABLE = CanReduceLp()>
   typename std::enable_if<ENABLE>::type
   ReduceLpImpl(LpInterfaceAdapter* l) const
   {
           factor_.ReduceLp(l, factor_); 
   }  
   void ReduceLp(LpInterfaceAdapter* l) const {
           ReduceLpImpl(l);
   }
  
   constexpr static bool CanCallGetNumberOfAuxVariables()
   {
      return FunctionExistence::HasGetNumberOfAuxVariables<FactorType,INDEX>();
   }
   template<bool ENABLE = CanCallGetNumberOfAuxVariables()>
   typename std::enable_if<!ENABLE,INDEX>::type
   GetNumberOfAuxVariablesImpl() const
   {
      return 0;
   }
   template<bool ENABLE = CanCallGetNumberOfAuxVariables()>
   typename std::enable_if<ENABLE,INDEX>::type
   GetNumberOfAuxVariablesImpl() const
   {
      return factor_.GetNumberOfAuxVariables();
   }
  
   INDEX GetNumberOfAuxVariables() const 
   { 
    return GetNumberOfAuxVariablesImpl();
   }

   void CreateConstraints(LpInterfaceAdapter* l) const final
   {
      static_if<CanCreateConstraints()>([&](auto f) {
            f(factor_).CreateConstraints(l);
      }).else_([&](auto f) {
         throw std::runtime_error("create constraints not implemented by factor");
      });
   }
};


} // end namespace LP_MP

#endif // LP_MP_FACTORS_MESSAGES_HXX

