
#include "r_rtsp/r_status.h"
#include "r_rtsp/r_exception.h"

using namespace r_rtsp;
using namespace r_utils;
using namespace std;

const static int S_UNDEFINED = 600;

string r_rtsp::get_status_message( status s )
{
    switch( s )
    {
    case STATUS_CONTINUE:
        return string("Continue");
        break;
    case STATUS_OK:
        return string("OK");
        break;
    case STATUS_CREATED:
        return string("Created");
        break;
    case STATUS_LOW_ON_STORAGE_SPACE:
        return string("Low on Storage Space");
        break;
    case STATUS_MULTIPLE_CHOICES:
        return string("Multiple Choices");
        break;
    case STATUS_MOVED_PERMANENTLY:
        return string("Moved Permanently");
        break;
    case STATUS_MOVED_TEMPORARILY:
        return string("Moved Temporarily");
        break;
    case STATUS_SEE_OTHER:
        return string("See Other");
        break;
    case STATUS_NOT_MODIFIED:
        return string("Not Modified");
        break;
    case STATUS_USE_PROXY:
        return string("Use Proxy");
        break;
    case STATUS_BAD_REQUEST:
        return string("Bad Request");
        break;
    case STATUS_UNAUTHORIZED:
        return string("Unauthorized");
        break;
    case STATUS_PAYMENT_REQUIRED:
        return string("Payment Required");
        break;
    case STATUS_FORBIDDEN:
        return string("Forbidden");
        break;
    case STATUS_NOT_FOUND:
        return string("Not Found");
        break;
    case STATUS_METHOD_NOT_ALLOWED:
        return string("Method Not Allowed");
        break;
    case STATUS_NOT_ACCEPTABLE:
        return string("Not Acceptable");
        break;
    case STATUS_PROXY_AUTH_REQUIRED:
        return string("Proxy Authentication Required");
        break;
    case STATUS_REQUEST_TIME_OUT:
        return string("Request Time-out");
        break;
    case STATUS_GONE:
        return string("Gone");
        break;
    case STATUS_LENGTH_REQUIRED:
        return string("Length Required");
        break;
    case STATUS_PRECONDITION_FAILED:
        return string("Precondition Failed");
        break;
    case STATUS_REQUEST_ENTITY_TOO_LARGE:
        return string("Request Entity Too Large");
        break;
    case STATUS_REQUEST_URI_TOO_LARGE:
        return string("Request-URI Too Large");
        break;
    case STATUS_UNSUPPORTED_MEDIA_TYPE:
        return string("Unsupported Media Type");
        break;
    case STATUS_PARAMETER_NOT_UNDERSTOOD:
        return string("Parameter Not Understood");
        break;
    case STATUS_CONFERENCE_NOT_FOUND:
        return string("Conference Not Found");
        break;
    case STATUS_NOT_ENOUGH_BANDWIDTH:
        return string("Not Enough Bandwidth");
        break;
    case STATUS_SESSION_NOT_FOUND:
        return string("Session Not Found");
        break;
    case STATUS_METHOD_NOT_VALID_IN_THIS_STATE:
        return string("Method Not Valid in This State");
        break;
    case STATUS_HEADER_FIELD_NOT_VALID_FOR_RESOURCE:
        return string("Header Field Not Valid for Resource");
        break;
    case STATUS_INVALID_RANGE:
        return string("Invalid Range");
        break;
    case STATUS_PARAMETER_IS_READONLY:
        return string("Parameter Is Read-Only");
        break;
    case STATUS_AGGREGATE_OPERATION_NOT_ALLOWED:
        return string("Aggregate operation not allowed");
        break;
    case STATUS_ONLY_AGGREGATE_OPERATION_ALLOWED:
        return string("Only aggregate operation allowed");
        break;
    case STATUS_UNSUPPORTED_TRANSPORT:
        return string("Unsupported transport");
        break;
    case STATUS_DESTINATION_UNREACHABLE:
        return string("Destination unreachable");
        break;
    case STATUS_INTERNAL_SERVER_ERROR:
        return string("Internal Server Error");
        break;
    case STATUS_NOT_IMPLEMENTED:
        return string("Not Implemented");
        break;
    case STATUS_BAD_GATEWAY:
        return string("Bad Gateway");
        break;
    case STATUS_SERVICE_UNAVAILABLE:
        return string("Service Unavailable");
        break;
    case STATUS_GATEWAY_TIMEOUT:
        return string("Gateway Time-out");
        break;
    case STATUS_RTSP_VERSION_NOT_SUPPORTED:
        return string("RTSP Version not supported");
        break;
    case STATUS_OPERATION_NOT_SUPPORTED:
        return string("Option not supported");
        break;
    default:
        break;
    };

    return string( "Unknown" );
}

status r_rtsp::convert_status_code_from_int( int value )
{
    if ( value == STATUS_CONTINUE )
        return (status)value;
    if ( value == STATUS_OK )
        return (status)value;
    if ( value == STATUS_CREATED )
        return (status)value;
    if ( value == STATUS_LOW_ON_STORAGE_SPACE )
        return (status)value;
    if ( value >= STATUS_MULTIPLE_CHOICES && value <= STATUS_USE_PROXY )
        return (status)value;
    if ( value >= STATUS_BAD_REQUEST && value <= STATUS_REQUEST_TIME_OUT )
        return (status)value;
    if ( value == STATUS_GONE )
        return (status)value;
    if ( value >= STATUS_LENGTH_REQUIRED && value <= STATUS_UNSUPPORTED_MEDIA_TYPE )
        return (status)value;
    if ( value >= STATUS_PARAMETER_NOT_UNDERSTOOD && value <= STATUS_DESTINATION_UNREACHABLE )
        return (status)value;
    if ( value >= STATUS_INTERNAL_SERVER_ERROR && value <= STATUS_RTSP_VERSION_NOT_SUPPORTED )
        return (status)value;
    if ( value == STATUS_OPERATION_NOT_SUPPORTED )
        return (status)value;
    if ( value >= STATUS_BAD_REQUEST && value < STATUS_INTERNAL_SERVER_ERROR )
        return STATUS_BAD_REQUEST;
    if ( value >= STATUS_INTERNAL_SERVER_ERROR && value < S_UNDEFINED )
        return STATUS_INTERNAL_SERVER_ERROR;
    if ( value >= STATUS_OK && value < STATUS_MULTIPLE_CHOICES )
        return STATUS_OK;

    R_STHROW(r_rtsp_exception, ("Value(%d) is not a valid status code!",value));
}
