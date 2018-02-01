// Generated by gencpp from file vrep_common/simRosGetAndClearStringSignal.msg
// DO NOT EDIT!


#ifndef VREP_COMMON_MESSAGE_SIMROSGETANDCLEARSTRINGSIGNAL_H
#define VREP_COMMON_MESSAGE_SIMROSGETANDCLEARSTRINGSIGNAL_H

#include <ros/service_traits.h>


#include <vrep_common/simRosGetAndClearStringSignalRequest.h>
#include <vrep_common/simRosGetAndClearStringSignalResponse.h>


namespace vrep_common
{

struct simRosGetAndClearStringSignal
{

typedef simRosGetAndClearStringSignalRequest Request;
typedef simRosGetAndClearStringSignalResponse Response;
Request request;
Response response;

typedef Request RequestType;
typedef Response ResponseType;

}; // struct simRosGetAndClearStringSignal
} // namespace vrep_common


namespace ros
{
namespace service_traits
{


template<>
struct MD5Sum< ::vrep_common::simRosGetAndClearStringSignal > {
  static const char* value()
  {
    return "2404c94dd63ca66111460f0d88cb7f64";
  }

  static const char* value(const ::vrep_common::simRosGetAndClearStringSignal&) { return value(); }
};

template<>
struct DataType< ::vrep_common::simRosGetAndClearStringSignal > {
  static const char* value()
  {
    return "vrep_common/simRosGetAndClearStringSignal";
  }

  static const char* value(const ::vrep_common::simRosGetAndClearStringSignal&) { return value(); }
};


// service_traits::MD5Sum< ::vrep_common::simRosGetAndClearStringSignalRequest> should match 
// service_traits::MD5Sum< ::vrep_common::simRosGetAndClearStringSignal > 
template<>
struct MD5Sum< ::vrep_common::simRosGetAndClearStringSignalRequest>
{
  static const char* value()
  {
    return MD5Sum< ::vrep_common::simRosGetAndClearStringSignal >::value();
  }
  static const char* value(const ::vrep_common::simRosGetAndClearStringSignalRequest&)
  {
    return value();
  }
};

// service_traits::DataType< ::vrep_common::simRosGetAndClearStringSignalRequest> should match 
// service_traits::DataType< ::vrep_common::simRosGetAndClearStringSignal > 
template<>
struct DataType< ::vrep_common::simRosGetAndClearStringSignalRequest>
{
  static const char* value()
  {
    return DataType< ::vrep_common::simRosGetAndClearStringSignal >::value();
  }
  static const char* value(const ::vrep_common::simRosGetAndClearStringSignalRequest&)
  {
    return value();
  }
};

// service_traits::MD5Sum< ::vrep_common::simRosGetAndClearStringSignalResponse> should match 
// service_traits::MD5Sum< ::vrep_common::simRosGetAndClearStringSignal > 
template<>
struct MD5Sum< ::vrep_common::simRosGetAndClearStringSignalResponse>
{
  static const char* value()
  {
    return MD5Sum< ::vrep_common::simRosGetAndClearStringSignal >::value();
  }
  static const char* value(const ::vrep_common::simRosGetAndClearStringSignalResponse&)
  {
    return value();
  }
};

// service_traits::DataType< ::vrep_common::simRosGetAndClearStringSignalResponse> should match 
// service_traits::DataType< ::vrep_common::simRosGetAndClearStringSignal > 
template<>
struct DataType< ::vrep_common::simRosGetAndClearStringSignalResponse>
{
  static const char* value()
  {
    return DataType< ::vrep_common::simRosGetAndClearStringSignal >::value();
  }
  static const char* value(const ::vrep_common::simRosGetAndClearStringSignalResponse&)
  {
    return value();
  }
};

} // namespace service_traits
} // namespace ros

#endif // VREP_COMMON_MESSAGE_SIMROSGETANDCLEARSTRINGSIGNAL_H
