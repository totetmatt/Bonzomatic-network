#include <stdio.h>
#include <string>
#include <vector>

#include "mongoose.h"

struct Room {
	std::string RoomName;
};

struct User {
	Room* CurrentRoom;
	std::string UserName;
};

std::vector<Room*> Rooms;
std::vector<User*> Users;

static sig_atomic_t s_signal_received = 0;
static const char *s_http_port = "8000";
static struct mg_serve_http_opts s_http_server_opts;

static void signal_handler(int sig_num) {
	signal(sig_num, signal_handler);  // Reinstantiate signal handler
	s_signal_received = sig_num;
}

static int is_websocket(const struct mg_connection *nc) {
	return nc->flags & MG_F_IS_WEBSOCKET;
}

static void broadcast(struct mg_connection *nc, const struct mg_str msg) {
	
	User* CurUser = (User*)nc->user_data;
  
    /*
	char addr[32];
	mg_sock_addr_to_str(&nc->sa, addr, sizeof(addr),
		MG_SOCK_STRINGIFY_IP | MG_SOCK_STRINGIFY_PORT);

	printf("Message from %s\n", addr);
    */

	printf("%s | Message from %s\n", CurUser->CurrentRoom->RoomName.c_str(), CurUser->UserName.c_str());

  //char buf[500];
	//snprintf(buf, sizeof(buf), "%s %.*s", addr, (int)msg.len, msg.p);
	//snprintf(buf, sizeof(buf), "*s", msg.p);
	//printf("%s\n", buf);

	struct mg_connection *c;
	for (c = mg_next(nc->mgr, NULL); c != NULL; c = mg_next(nc->mgr, c)) {
		if (c == nc) continue; /* Don't send to the sender. */
		User* TargetUser = (User*)c->user_data;
    // Only send if in the same room
		if (TargetUser && TargetUser->CurrentRoom == CurUser->CurrentRoom) {
			mg_send_websocket_frame(c, WEBSOCKET_OP_TEXT, msg.p, msg.len);
	  }
	}
}

static void ev_handler(struct mg_connection *nc, int ev, void *ev_data) {
	switch (ev) {
	case MG_EV_WEBSOCKET_HANDSHAKE_DONE: {
		/* New websocket connection. Tell everybody. */
		//broadcast(nc, mg_mk_str("++ joined"));
		char addr[32];
		mg_sock_addr_to_str(&nc->sa, addr, sizeof(addr), MG_SOCK_STRINGIFY_IP | MG_SOCK_STRINGIFY_PORT | MG_SOCK_STRINGIFY_REMOTE);

		struct http_message *hm = (struct http_message *) ev_data;
		std::string joinedRoom;
		if (hm->uri.len == 0 || hm->uri.p[0] != '/') {
			joinedRoom = "DefaultRoom";
		} else {
		  joinedRoom.append(hm->uri.p, hm->uri.len);
	  }
		
		Room* TargetRoom = NULL;
		for (int i = 0; i < Rooms.size(); ++i) {
			if (Rooms[i]->RoomName == joinedRoom) {
				//printf("Found room %s\n", joinedRoom.c_str());
				TargetRoom = Rooms[i];
				break;
	    }
	  }
		if (!TargetRoom) { // no room found
			Room* NewRoom = new Room();
			NewRoom->RoomName = joinedRoom;
			Rooms.push_back(NewRoom);
			TargetRoom = NewRoom;
	  }
		
		User* NewUser = new User();
		NewUser->UserName = addr;
		NewUser->CurrentRoom = TargetRoom;
		Users.push_back(NewUser);

		nc->user_data = NewUser;
		printf("%s ++ joined %s\n", addr, joinedRoom.c_str());
		break;
	}
	case MG_EV_WEBSOCKET_FRAME: {
		struct websocket_message *wm = (struct websocket_message *) ev_data;
		/* New websocket message. Tell everybody. */
		struct mg_str d = { (char *)wm->data, wm->size };
		broadcast(nc, d);
		break;
	}
	case MG_EV_HTTP_REQUEST: {
		mg_serve_http(nc, (struct http_message *) ev_data, s_http_server_opts);
		break;
	}
	case MG_EV_CLOSE: {
		/* Disconnect. Tell everybody. */
		if (is_websocket(nc)) {
			//broadcast(nc, mg_mk_str("-- left"));
			char addr[32];
			mg_sock_addr_to_str(&nc->sa, addr, sizeof(addr), MG_SOCK_STRINGIFY_IP | MG_SOCK_STRINGIFY_PORT);

			for (auto it = begin(Users); it != end(Users); ++it) {
				User* Cur = (*it);
				if (Cur == nc->user_data) {
					//printf("Found %s\n", Cur->CurrentRoom->RoomName.c_str());
					Users.erase(it);
					delete Cur;
					break;
		    }
			}

			printf("%s -- left\n", addr);
		}
		break;
	}
	}
}

int main(int argc, const char *argv[])
{
	printf("[SERVER] Started \n");

	struct mg_mgr mgr;
	struct mg_connection *nc;
    
	signal(SIGTERM, signal_handler);
	signal(SIGINT, signal_handler);
	//setvbuf(stdout, NULL, _IOLBF, 0);
	//setvbuf(stderr, NULL, _IOLBF, 0);

    
	mg_mgr_init(&mgr, NULL);

	nc = mg_bind(&mgr, s_http_port, ev_handler);
	mg_set_protocol_http_websocket(nc);
	s_http_server_opts.document_root = ".";  // Serve current directory
	s_http_server_opts.enable_directory_listing = "yes";

	printf("Started on port %s\n", s_http_port);
	while (s_signal_received == 0) {
		mg_mgr_poll(&mgr, 200);
	}
	mg_mgr_free(&mgr);
    

	// Wait indefinitely
	while (true) {
		Sleep(30);
	}

  // Clean
	for (auto Cur : Rooms) {
		delete Cur;
	}
	Rooms.clear();

	return 0;
}
