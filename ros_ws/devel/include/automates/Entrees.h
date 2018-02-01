// Generated by gencpp from file automates/Entrees.msg
// DO NOT EDIT!


#ifndef AUTOMATES_MESSAGE_ENTREES_H
#define AUTOMATES_MESSAGE_ENTREES_H


#include <string>
#include <vector>
#include <map>

#include <ros/types.h>
#include <ros/serialization.h>
#include <ros/builtin_message_traits.h>
#include <ros/message_operations.h>


namespace automates
{
template <class ContainerAllocator>
struct Entrees_
{
  typedef Entrees_<ContainerAllocator> Type;

  Entrees_()
    : entrees(0)  {
    }
  Entrees_(const ContainerAllocator& _alloc)
    : entrees(0)  {
  (void)_alloc;
    }



   typedef int32_t _entrees_type;
  _entrees_type entrees;




  typedef boost::shared_ptr< ::automates::Entrees_<ContainerAllocator> > Ptr;
  typedef boost::shared_ptr< ::automates::Entrees_<ContainerAllocator> const> ConstPtr;

}; // struct Entrees_

typedef ::automates::Entrees_<std::allocator<void> > Entrees;

typedef boost::shared_ptr< ::automates::Entrees > EntreesPtr;
typedef boost::shared_ptr< ::automates::Entrees const> EntreesConstPtr;

// constants requiring out of line definition



template<typename ContainerAllocator>
std::ostream& operator<<(std::ostream& s, const ::automates::Entrees_<ContainerAllocator> & v)
{
ros::message_operations::Printer< ::automates::Entrees_<ContainerAllocator> >::stream(s, "", v);
return s;
}

} // namespace automates

namespace ros
{
namespace message_traits
{



// BOOLTRAITS {'IsFixedSize': True, 'IsMessage': True, 'HasHeader': False}
// {'automates': ['/home/projn7cellule/Working_Folder_Florent/ros_ws/src/automates/msg'], 'std_msgs': ['/opt/ros/indigo/share/std_msgs/cmake/../msg']}

// !!!!!!!!!!! ['__class__', '__delattr__', '__dict__', '__doc__', '__eq__', '__format__', '__getattribute__', '__hash__', '__init__', '__module__', '__ne__', '__new__', '__reduce__', '__reduce_ex__', '__repr__', '__setattr__', '__sizeof__', '__str__', '__subclasshook__', '__weakref__', '_parsed_fields', 'constants', 'fields', 'full_name', 'has_header', 'header_present', 'names', 'package', 'parsed_fields', 'short_name', 'text', 'types']




template <class ContainerAllocator>
struct IsFixedSize< ::automates::Entrees_<ContainerAllocator> >
  : TrueType
  { };

template <class ContainerAllocator>
struct IsFixedSize< ::automates::Entrees_<ContainerAllocator> const>
  : TrueType
  { };

template <class ContainerAllocator>
struct IsMessage< ::automates::Entrees_<ContainerAllocator> >
  : TrueType
  { };

template <class ContainerAllocator>
struct IsMessage< ::automates::Entrees_<ContainerAllocator> const>
  : TrueType
  { };

template <class ContainerAllocator>
struct HasHeader< ::automates::Entrees_<ContainerAllocator> >
  : FalseType
  { };

template <class ContainerAllocator>
struct HasHeader< ::automates::Entrees_<ContainerAllocator> const>
  : FalseType
  { };


template<class ContainerAllocator>
struct MD5Sum< ::automates::Entrees_<ContainerAllocator> >
{
  static const char* value()
  {
    return "2fd94acaf54653c950d522c68a591eb3";
  }

  static const char* value(const ::automates::Entrees_<ContainerAllocator>&) { return value(); }
  static const uint64_t static_value1 = 0x2fd94acaf54653c9ULL;
  static const uint64_t static_value2 = 0x50d522c68a591eb3ULL;
};

template<class ContainerAllocator>
struct DataType< ::automates::Entrees_<ContainerAllocator> >
{
  static const char* value()
  {
    return "automates/Entrees";
  }

  static const char* value(const ::automates::Entrees_<ContainerAllocator>&) { return value(); }
};

template<class ContainerAllocator>
struct Definition< ::automates::Entrees_<ContainerAllocator> >
{
  static const char* value()
  {
    return "int32 entrees\n\
\n\
";
  }

  static const char* value(const ::automates::Entrees_<ContainerAllocator>&) { return value(); }
};

} // namespace message_traits
} // namespace ros

namespace ros
{
namespace serialization
{

  template<class ContainerAllocator> struct Serializer< ::automates::Entrees_<ContainerAllocator> >
  {
    template<typename Stream, typename T> inline static void allInOne(Stream& stream, T m)
    {
      stream.next(m.entrees);
    }

    ROS_DECLARE_ALLINONE_SERIALIZER
  }; // struct Entrees_

} // namespace serialization
} // namespace ros

namespace ros
{
namespace message_operations
{

template<class ContainerAllocator>
struct Printer< ::automates::Entrees_<ContainerAllocator> >
{
  template<typename Stream> static void stream(Stream& s, const std::string& indent, const ::automates::Entrees_<ContainerAllocator>& v)
  {
    s << indent << "entrees: ";
    Printer<int32_t>::stream(s, indent + "  ", v.entrees);
  }
};

} // namespace message_operations
} // namespace ros

#endif // AUTOMATES_MESSAGE_ENTREES_H
