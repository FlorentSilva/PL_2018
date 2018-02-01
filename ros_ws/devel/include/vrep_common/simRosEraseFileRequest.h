// Generated by gencpp from file vrep_common/simRosEraseFileRequest.msg
// DO NOT EDIT!


#ifndef VREP_COMMON_MESSAGE_SIMROSERASEFILEREQUEST_H
#define VREP_COMMON_MESSAGE_SIMROSERASEFILEREQUEST_H


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
struct simRosEraseFileRequest_
{
  typedef simRosEraseFileRequest_<ContainerAllocator> Type;

  simRosEraseFileRequest_()
    : fileName()  {
    }
  simRosEraseFileRequest_(const ContainerAllocator& _alloc)
    : fileName(_alloc)  {
  (void)_alloc;
    }



   typedef std::basic_string<char, std::char_traits<char>, typename ContainerAllocator::template rebind<char>::other >  _fileName_type;
  _fileName_type fileName;




  typedef boost::shared_ptr< ::vrep_common::simRosEraseFileRequest_<ContainerAllocator> > Ptr;
  typedef boost::shared_ptr< ::vrep_common::simRosEraseFileRequest_<ContainerAllocator> const> ConstPtr;

}; // struct simRosEraseFileRequest_

typedef ::vrep_common::simRosEraseFileRequest_<std::allocator<void> > simRosEraseFileRequest;

typedef boost::shared_ptr< ::vrep_common::simRosEraseFileRequest > simRosEraseFileRequestPtr;
typedef boost::shared_ptr< ::vrep_common::simRosEraseFileRequest const> simRosEraseFileRequestConstPtr;

// constants requiring out of line definition



template<typename ContainerAllocator>
std::ostream& operator<<(std::ostream& s, const ::vrep_common::simRosEraseFileRequest_<ContainerAllocator> & v)
{
ros::message_operations::Printer< ::vrep_common::simRosEraseFileRequest_<ContainerAllocator> >::stream(s, "", v);
return s;
}

} // namespace vrep_common

namespace ros
{
namespace message_traits
{



// BOOLTRAITS {'IsFixedSize': False, 'IsMessage': True, 'HasHeader': False}
// {'sensor_msgs': ['/opt/ros/indigo/share/sensor_msgs/cmake/../msg'], 'std_msgs': ['/opt/ros/indigo/share/std_msgs/cmake/../msg'], 'geometry_msgs': ['/opt/ros/indigo/share/geometry_msgs/cmake/../msg'], 'vrep_common': ['/home/projn7cellule/PL_2018/ros_ws/src/vrep_common/msg']}

// !!!!!!!!!!! ['__class__', '__delattr__', '__dict__', '__doc__', '__eq__', '__format__', '__getattribute__', '__hash__', '__init__', '__module__', '__ne__', '__new__', '__reduce__', '__reduce_ex__', '__repr__', '__setattr__', '__sizeof__', '__str__', '__subclasshook__', '__weakref__', '_parsed_fields', 'constants', 'fields', 'full_name', 'has_header', 'header_present', 'names', 'package', 'parsed_fields', 'short_name', 'text', 'types']




template <class ContainerAllocator>
struct IsFixedSize< ::vrep_common::simRosEraseFileRequest_<ContainerAllocator> >
  : FalseType
  { };

template <class ContainerAllocator>
struct IsFixedSize< ::vrep_common::simRosEraseFileRequest_<ContainerAllocator> const>
  : FalseType
  { };

template <class ContainerAllocator>
struct IsMessage< ::vrep_common::simRosEraseFileRequest_<ContainerAllocator> >
  : TrueType
  { };

template <class ContainerAllocator>
struct IsMessage< ::vrep_common::simRosEraseFileRequest_<ContainerAllocator> const>
  : TrueType
  { };

template <class ContainerAllocator>
struct HasHeader< ::vrep_common::simRosEraseFileRequest_<ContainerAllocator> >
  : FalseType
  { };

template <class ContainerAllocator>
struct HasHeader< ::vrep_common::simRosEraseFileRequest_<ContainerAllocator> const>
  : FalseType
  { };


template<class ContainerAllocator>
struct MD5Sum< ::vrep_common::simRosEraseFileRequest_<ContainerAllocator> >
{
  static const char* value()
  {
    return "0c96d4662a80949a4927729271153923";
  }

  static const char* value(const ::vrep_common::simRosEraseFileRequest_<ContainerAllocator>&) { return value(); }
  static const uint64_t static_value1 = 0x0c96d4662a80949aULL;
  static const uint64_t static_value2 = 0x4927729271153923ULL;
};

template<class ContainerAllocator>
struct DataType< ::vrep_common::simRosEraseFileRequest_<ContainerAllocator> >
{
  static const char* value()
  {
    return "vrep_common/simRosEraseFileRequest";
  }

  static const char* value(const ::vrep_common::simRosEraseFileRequest_<ContainerAllocator>&) { return value(); }
};

template<class ContainerAllocator>
struct Definition< ::vrep_common::simRosEraseFileRequest_<ContainerAllocator> >
{
  static const char* value()
  {
    return "\n\
\n\
\n\
\n\
string fileName\n\
";
  }

  static const char* value(const ::vrep_common::simRosEraseFileRequest_<ContainerAllocator>&) { return value(); }
};

} // namespace message_traits
} // namespace ros

namespace ros
{
namespace serialization
{

  template<class ContainerAllocator> struct Serializer< ::vrep_common::simRosEraseFileRequest_<ContainerAllocator> >
  {
    template<typename Stream, typename T> inline static void allInOne(Stream& stream, T m)
    {
      stream.next(m.fileName);
    }

    ROS_DECLARE_ALLINONE_SERIALIZER
  }; // struct simRosEraseFileRequest_

} // namespace serialization
} // namespace ros

namespace ros
{
namespace message_operations
{

template<class ContainerAllocator>
struct Printer< ::vrep_common::simRosEraseFileRequest_<ContainerAllocator> >
{
  template<typename Stream> static void stream(Stream& s, const std::string& indent, const ::vrep_common::simRosEraseFileRequest_<ContainerAllocator>& v)
  {
    s << indent << "fileName: ";
    Printer<std::basic_string<char, std::char_traits<char>, typename ContainerAllocator::template rebind<char>::other > >::stream(s, indent + "  ", v.fileName);
  }
};

} // namespace message_operations
} // namespace ros

#endif // VREP_COMMON_MESSAGE_SIMROSERASEFILEREQUEST_H
