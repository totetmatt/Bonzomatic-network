
#include "mongoose.h"
#include "Network.h"
#include "jsonxx.h"
#include <map>

#include "Instances.h"

namespace Network {

  static bool NetworkLaunched = false;
  
static int s_done = 0;
static int s_is_connected = 0;

void RecieveShader(size_t size, unsigned char *data);

static void ev_handler(struct mg_connection *nc, int ev, void *ev_data) {
  (void) nc;

  switch (ev) {
    case MG_EV_CONNECT: {
      int status = *((int *) ev_data);
      if (status != 0) {
        printf("-- Connection error: %d\n", status);
      } else {
      }
      break;
    }
    case MG_EV_WEBSOCKET_HANDSHAKE_DONE: {
      struct http_message *hm = (struct http_message *) ev_data;
      if (hm->resp_code == 101) {
        printf("-- Connected\n");
        s_is_connected = 1;
      } else {
        printf("-- Connection failed! HTTP code %d\n", hm->resp_code);
        /* Connection will be closed after this. */
      }
      break;
    }
    case MG_EV_POLL: {
      // nothing to send
      break;
    }
    case MG_EV_WEBSOCKET_FRAME: {
      struct websocket_message *wm = (struct websocket_message *) ev_data;
      //printf("%.*s\n", (int) wm->size, wm->data);
			//printf("Grabbed message %d.\n", (int)wm->size);
			RecieveShader(wm->size, wm->data);
      break;
    }
    case MG_EV_CLOSE: {
      if (s_is_connected) printf("-- Disconnected\n");
      s_done = 1;
      break;
    }
  }
}
struct mg_mgr mgr;
struct mg_connection *nc;

int CurrentFrame = 0;

void PrepareConnection()
{
  CurrentFrame = 0;
  s_done = 0;
  s_is_connected = 0;
  mg_mgr_init(&mgr, NULL);
}

void OpenConnection(std::string ServerURL)
{
  nc = mg_connect_ws(&mgr, ev_handler, ServerURL.c_str(), "ws_chat", NULL);
  if (nc == NULL) {
    fprintf(stderr, "Invalid address\n");
    return;
  }
  NetworkLaunched = true;
}

void RecieveShader(size_t size, unsigned char *data) {
  
	// TODO: very very bad, we should:
	// - use json
	// - verify size
	// - non-ascii symbols ?
	// - asynchronous update ?
	data[size-1] = '\0'; // ensure we do have an end to the char string after "size" bytes
	std::string TextJson = std::string((char*)data);
	//printf("JSON: %s\n", TextJson.c_str());
	jsonxx::Object NewShader;
	bool ErrorFound = false;
	if (NewShader.parse(TextJson)) {
		if(NewShader.has<jsonxx::Object>("Data")) {
			jsonxx::Object Data = NewShader.get<jsonxx::Object>("Data");
      if (!Data.has<jsonxx::String>("RoomName")) ErrorFound = true;
      if (!Data.has<jsonxx::String>("NickName")) ErrorFound = true;
		} else {
			ErrorFound = true;
		}
	} else {
		ErrorFound = true;
	}
	if(ErrorFound) {
		printf("Invalid json formatting\n");
		return;
	}

  jsonxx::Object Data = NewShader.get<jsonxx::Object>("Data");
  std::string RoomName = Data.get<jsonxx::String>("RoomName");
	std::string NickName = Data.get<jsonxx::String>("NickName");
  //std::string FullName = RoomName + "/" + NickName;
  SignalLiveUser(NickName);
}

void Tick(float ElapsedTime) {
  if (NetworkLaunched) {
    mg_mgr_poll(&mgr, 10);
    CurrentFrame++;
  }
}

void Release() {
  if (NetworkLaunched) {
    mg_mgr_free(&mgr);
    NetworkLaunched = false;
  }
}

bool IsLaunched() {
  return NetworkLaunched;
}

bool IsActive() { return s_is_connected && !s_done; }

}