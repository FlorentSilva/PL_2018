// Generated by gencpp from file vrep_common/simRosGetStringSignalResponse.msg
// DO NOT EDIT!


#ifndef VREP_COMMON_MESSAGE_SIMROSGETSTRINGSIGNALRESPONSE_H
#define VREP_COMMON_MESSAGE_SIMROSGETSTRINGSIGNALRESPONSE_H


#include <string>
#include <vector>
#include <map>

#include <ros/types.h>
#include <ros/serialization.h>
#include <ros/builtin_message_traits.h>
#include <ros/message_operations.h>


namespace vrep_common
{
template <class ContainerAllocator>
struct simRosGetStringSignalResponse_
{
  typedef simRosGetStringSignalResponse_<ContainerAllocator> Type;

  simRosGetStringSignalResponse_()
    : result(0)
    , signalValue()  {
    }
  simRosGetStringSignalResponse_(const ContainerAllocator& _alloc)
    : result(0)
    , signalValue(_alloc)  {
  (void)_alloc;
    }



   typedef int32_t _result_type;
  _result_type result;

   typedef std::basic_string<char, std::char_traits<char>, typename ContainerAllocator::template rebind<char>::other >  _signalValue_type;
  _signalValue_type signalValue;




  typedef boost::shared_ptr< ::vrep_common::simRosGetStringSignalResponse_<ContainerAllocator> > Ptr;
  typedef boost::shared_ptr< ::vrep_common::simRosGetStringSignalResponse_<ContainerAllocator> const> ConstPtr;

}; // struct simRosGetStringSignalResponse_

typedef ::vrep_common::simRosGetStringSignalResponse_<std::allocator<void> > simRosGetStringSignalResponse;

typedef boost::shared_ptr< ::vrep_common::simRosGetStringSignalResponse > simRosGetStringSignalResponsePtr;
typedef boost::shared_ptr< ::vrep_common::simRosGetStringSignalResponse const> simRosGetStringSignalResponseConstPtr;

// constants requiring out of line definition



template<typename ContainerAllocator>
std::ostream& operator<<(std::ostream& s, const ::vrep_common::simRosGetStringSignalResponse_<ContainerAllocator> & v)
{
ros::message_operations::Printer< ::vrep_common::simRosGetStringSignalResponse_<ContainerAllocator> >::stream(s, "", v);
return s;
}

} // namespace vrep_common

namespace ros
{
namespace message_traits
{



// BOOLTRAITS {'IsFixedSize': False, 'IsMessage': True, 'HasHeader': False}
// {'sensor_msgs': ['/opt/ros/indigo/share/sensor_msgs/cmake/../msg'], 'std_msgs': ['/opt/ros/indigo/share/std_msgs/cmake/../msg'], 'geometry_msgs': ['/opt/ros/indigo/share/geometry_msgs/cmake/../msg'], 'vrep_common': ['/home/projn7cellule/Working_Folder_Florent/ros_ws/src/vrep_common/msg']}

// !!!!!!!!!!! ['__class__', '__delattr__', '__dict__', '__doc__', '__eq__', '__format__', '__getattribute__', '__hash__', '__init__', '__module__', '__ne__', '__new__', '__reduce__', '__reduce_ex__', '__repr__', '__setattr__', '__sizeof__', '__str__', '__subclasshook__', '__weakref__', '_parsed_fields', 'constants', 'fields', 'full_name', 'has_header', 'header_present', 'names', 'package', 'parsed_fields', 'short_name', 'text', 'types']




template <class ContainerAllocator>
struct IsFixedSize< ::vrep_common::simRosGetStringSignalResponse_<ContainerAllocator> >
  : FalseType
  { };

template <class ContainerAllocator>
struct IsFixedSize< ::vrep_common::simRosGetStringSignalResponse_<ContainerAllocator> const>
  : FalseType
  { };

template <class ContainerAllocator>
struct IsMessage< ::vrep_common::simRosGetStringSignalResponse_<ContainerAllocator> >
  : TrueType
  { };

template <class ContainerAllocator>
struct IsMessage< ::vrep_common::simRosGetStringSignalResponse_<ContainerAllocator> const>
  : TrueType
  { };

template <class ContainerAllocator>
struct HasHeader< ::vrep_common::simRosGetStringSignalResponse_<ContainerAllocator> >
  : FalseType
  { };

template <class ContainerAllocator>
struct HasHeader< ::vrep_common::simRosGetStringSignalResponse_<ContainerAllocator> const>
  : FalseType
  { };


template<class ContainerAllocator>
struct MD5Sum< ::vrep_common::simRosGetStringSignalResponse_<ContainerAllocator> >
{
  static const char* value()
  {
    return "db26c269346c2452f7366e4e3eed9867";
  }

  static const char* value(const ::vrep_common::simRosGetStringSignalResponse_<ContainerAllocator>&) { return value(); }
  static const uint64_t static_value1 = 0xdb26c269346c2452ULL;
  static const uint64_t static_value2 = 0xf7366e4e3eed9867ULL;
};

template<class ContainerAllocator>
struct DataType< ::vrep_common::simRosGetStringSignalResponse_<ContainerAllocator> >
{
  static const char* value()
  {
    return "vrep_common/simRosGetStringSignalResponse";
  }

  static const char* value(const ::vrep_common::simRosGetStringSignalResponse_<ContainerAllocator>&) { return value(); }
};

template<class ContainerAllocator>
struct Definition< ::vrep_common::simRosGetStringSignalResponse_<ContainerAllocator> >
{
  static const char* value()
  {
    return "int32 result\n\
string signalValue\n\
\n\
";
  }

  static const char* value(const ::vrep_common::simRosGetStringSignalResponse_<ContainerAllocator>&) { return value(); }
};

} // namespace message_traits
} // namespace ros

namespace ros
{
namespace serialization
{

  template<class ContainerAllocator> struct Serializer< ::vrep_common::simRosGetStringSignalResponse_<ContainerAllocator> >
  {
    template<typename Stream, typename T> inline static void allInOne(Stream& stream, T m)
    {
      stream.next(m.result);
      stream.next(m.signalValue);
    }

    ROS_DECLARE_ALLINONE_SERIALIZER
  }; // struct simRosGetStringSignalResponse_

} // namespace serialization
} // namespace ros

namespace ros
{
namespace message_operations
{

template<class ContainerAllocator>
struct Printer< ::vrep_common::simRosGetStringSignalResponse_<ContainerAllocator> >
{
  template<typename Stream> static void stream(Stream& s, const std::string& indent, const ::vrep_common::simRosGetStringSignalResponse_<ContainerAllocator>& v)
  {
    s << indent << "result: ";
    Printer<int32_t>::stream(s, indent + "  ", v.result);
    s << indent << "signalValue: ";
    Printer<std::basic_string<char, std::char_traits<char>, typename ContainerAllocator::template rebind<char>::other > >::stream(s, indent + "  ", v.signalValue);
  }
};

} // namespace message_operations
} // namespace ros

#endif // VREP_COMMON_MESSAGE_SIMROSGETSTRINGSIGNALRESPONSE_H
