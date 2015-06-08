#ifndef __DNP_STATUS_H__
#define __DNP_STATUS_H__


enum dnp_status {
  DNP_SUCCESS   = 0x0000,
  DNP_MAIL_ERROR = 0x0001,
  DNP_NO_MAIL    = 0x0002,

};

typedef enum dnp_status dnp_status_t;




#endif /* __DNP_STATUS_H__ */
