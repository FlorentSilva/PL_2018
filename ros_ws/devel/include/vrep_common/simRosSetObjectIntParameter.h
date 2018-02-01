// Generated by gencpp from file vrep_common/simRosSetObjectIntParameter.msg
// DO NOT EDIT!


#ifndef VREP_COMMON_MESSAGE_SIMROSSETOBJECTINTPARAMETER_H
#define VREP_COMMON_MESSAGE_SIMROSSETOBJECTINTPARAMETER_H

#include <ros/service_traits.h>


#include <vrep_common/simRosSetObjectIntParameterRequest.h>
#include <vrep_common/simRosSetObjectIntParameterResponse.h>


namespace vrep_common
{

struct simRosSetObjectIntParameter
{

typedef simRosSetObjectIntParameterRequest Request;
typedef simRosSetObjectIntParameterResponse Response;
Request request;
Response response;

typedef Request RequestType;
typedef Response ResponseType;

}; // struct simRosSetObjectIntParameter
} // namespace vrep_common


namespace ros
{
namespace service_traits
{


template<>
struct MD5Sum< ::vrep_common::simRosSetObjectIntParameter > {
  static const char* value()
  {
    return "fd6c30a0a0bb1198c016550800028a13";
  }

  static const char* value(const ::vrep_common::simRosSetObjectIntParameter&) { return value(); }
};

template<>
struct DataType< ::vrep_common::simRosSetObjectIntParameter > {
  static const char* value()
  {
    return "vrep_common/simRosSetObjectIntParameter";
  }

  static const char* value(const ::vrep_common::simRosSetObjectIntParameter&) { return value(); }
};


// service_traits::MD5Sum< ::vrep_common::simRosSetObjectIntParameterRequest> should match 
// service_traits::MD5Sum< ::vrep_common::simRosSetObjectIntParameter > 
template<>
struct MD5Sum< ::vrep_common::simRosSetObjectIntParameterRequest>
{
  static const char* value()
  {
    return MD5Sum< ::vrep_common::simRosSetObjectIntParameter >::value();
  }
  static const char* value(const ::vrep_common::simRosSetObjectIntParameterRequest&)
  {
    return value();
  }
};

// service_traits::DataType< ::vrep_common::simRosSetObjectIntParameterRequest> should match 
// service_traits::DataType< ::vrep_common::simRosSetObjectIntParameter > 
template<>
struct DataType< ::vrep_common::simRosSetObjectIntParameterRequest>
{
  static const char* value()
  {
    return DataType< ::vrep_common::simRosSetObjectIntParameter >::value();
  }
  static const char* value(const ::vrep_common::simRosSetObjectIntParameterRequest&)
  {
    return value();
  }
};

// service_traits::MD5Sum< ::vrep_common::simRosSetObjectIntParameterResponse> should match 
// service_traits::MD5Sum< ::vrep_common::simRosSetObjectIntParameter > 
template<>
struct MD5Sum< ::vrep_common::simRosSetObjectIntParameterResponse>
{
  static const char* value()
  {
    return MD5Sum< ::vrep_common::simRosSetObjectIntParameter >::value();
  }
  static const char* value(const ::vrep_common::simRosSetObjectIntParameterResponse&)
  {
    return value();
  }
};

// service_traits::DataType< ::vrep_common::simRosSetObjectIntParameterResponse> should match 
// service_traits::DataType< ::vrep_common::simRosSetObjectIntParameter > 
template<>
struct DataType< ::vrep_common::simRosSetObjectIntParameterResponse>
{
  static const char* value()
  {
    return DataType< ::vrep_common::simRosSetObjectIntParameter >::value();
  }
  static const char* value(const ::vrep_common::simRosSetObjectIntParameterResponse&)
  {
    return value();
  }
};

} // namespace service_traits
} // namespace ros

#endif // VREP_COMMON_MESSAGE_SIMROSSETOBJECTINTPARAMETER_H
