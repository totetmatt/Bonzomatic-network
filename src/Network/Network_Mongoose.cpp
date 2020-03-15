#include <stdio.h>
#include "jsonxx.h"
#include "mongoose.h"
#include "../Cmdline.h"

namespace Network
{
	static int s_done = 0;
	static int s_is_connected = 0;

	bool NewShaderGrabber = false;
	std::string LastGrabberShader;

	enum NetMode {
		NetMode_Sender,
		NetMode_Grabber
	};

	bool bNetworkEnabled = false;
	bool bNetworkLaunched = false;
	std::string ServerURL;
	std::string NetworkModeString;
	NetMode NetworkMode = NetMode::NetMode_Sender;

	void RecieveShader(size_t size, unsigned char *data);

	static void ev_handler(struct mg_connection *nc, int ev, void *ev_data) {
		(void)nc;

		switch (ev) {
		case MG_EV_CONNECT: {
			int status = *((int *)ev_data);
			if (status != 0) {
				printf("-- Connection error: %d\n", status);
			}
			break;
		}
		case MG_EV_WEBSOCKET_HANDSHAKE_DONE: {
			struct http_message *hm = (struct http_message *) ev_data;
			if (hm->resp_code == 101) {
				printf("-- Connected\n");
				s_is_connected = 1;
				bNetworkLaunched = true;
			}
			else {
				printf("-- Connection failed! HTTP code %d\n", hm->resp_code);
				/* Connection will be closed after this. */
			}
			break;
		}
		case MG_EV_POLL: {
			//char msg[500];
			//strcpy(msg, "Bonjour de loin");
			//mg_send_websocket_frame(nc, WEBSOCKET_OP_TEXT, msg, strlen(msg));
			break;
		}
		case MG_EV_WEBSOCKET_FRAME: {
			struct websocket_message *wm = (struct websocket_message *) ev_data;
		printf("Grabbed message %d.\n", (int)wm->size);
		RecieveShader(wm->size, wm->data);
			
			break;
		}
		case MG_EV_CLOSE: {
			if (s_is_connected) printf("-- Disconnected\n");
			s_done = 1;
			bNetworkLaunched = false;
			break;
		}
		}
	}

	struct mg_mgr mgr;
	struct mg_connection *nc;
  
	void LoadSettings(jsonxx::Object & o)
	{
		if (o.has<jsonxx::Object>("network"))
		{
			jsonxx::Object netjson = o.get<jsonxx::Object>("network");
			if (netjson.has<jsonxx::Boolean>("enabled"))
				bNetworkEnabled = netjson.get<jsonxx::Boolean>("enabled");
			if (netjson.has<jsonxx::String>("serverURL"))
				ServerURL = netjson.get<jsonxx::String>("serverURL");
			if (netjson.has<jsonxx::String>("networkMode"))
				NetworkModeString = netjson.get<jsonxx::String>("networkMode");
		}
	}

	void CommandLine(int argc, const char *argv[]) {
		if (CmdHasOption(argc, argv, "serverURL", &ServerURL)) {
			bNetworkEnabled = true;
			printf("Set server URL to %s \n", ServerURL.c_str());
		}
		if (CmdHasOption(argc, argv, "networkMode", &NetworkModeString)) {
			bNetworkEnabled = true;
			printf("Set server mode to %s \n", NetworkModeString.c_str());
		}
	}

	void OpenConnection()
	{
		if (!bNetworkEnabled) return;
		
		if (NetworkModeString == "grabber") {
			NetworkMode = NetMode_Grabber;
		}

		if (NetworkMode == NetMode_Grabber) {
			printf("Network Launching as grabber\n");
		} else {
			printf("Network Launching as sender\n");
		}

		mg_mgr_init(&mgr, NULL);

		// ws://127.0.0.1:8000
		printf("Try to connect to %s\n", ServerURL.c_str());
		nc = mg_connect_ws(&mgr, ev_handler, ServerURL.c_str(), "ws_chat", NULL);
		if (nc == NULL) {
			fprintf(stderr, "Invalid address\n");
			return;
		}
	}

	void BroadcastMessage(const char* msg) {
		if (!bNetworkLaunched) return;

		mg_send_websocket_frame(nc, WEBSOCKET_OP_TEXT, msg, strlen(msg)+1);
	}

	void SendShader(const char* msg) {
		if (!bNetworkLaunched) return;
		if (NetworkMode != NetMode_Sender) return;

		mg_send_websocket_frame(nc, WEBSOCKET_OP_TEXT, msg, strlen(msg)+1);
	}

	void RecieveShader(size_t size, unsigned char *data) {
		// TODO: very very bad, we should:
		// - use json
		// - verify size
		// - non-ascii symbols ?
		// - asynchronous update ?
		data[size-1] = '\0';
		LastGrabberShader = std::string((char*)data);
		NewShaderGrabber = true;
	}

	bool IsConnected() {
		return bNetworkLaunched;
	}

	bool IsGrabber() {
		return NetworkMode == NetMode_Grabber;
	}

	bool HasNewShader() {
		if (NetworkMode != NetMode_Grabber) return false;
		return NewShaderGrabber;
	}

	std::string GetNewShader() {
		if (NetworkMode != NetMode_Grabber) return std::string();
		NewShaderGrabber = false;
		return LastGrabberShader;
	}

	void Tick() {
		if (!bNetworkEnabled) return;

		// TODO: should be in another thread?
		mg_mgr_poll(&mgr, 10);
	}

	void Release() {
		if (!bNetworkLaunched) return;

		mg_mgr_free(&mgr);
	}
}