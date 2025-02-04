#include "Processor/ExternalClients.h"
#include "Networking/ServerSocket.h"
#include <netinet/in.h>
#include <arpa/inet.h>
#include <thread>

ExternalClients::ExternalClients(int party_num, const string& prep_data_dir):
   party_num(party_num), prep_data_dir(prep_data_dir),
   ctx(0)
{
}

ExternalClients::~ExternalClients() 
{
  // close client sockets
  for (auto it = external_client_sockets.begin();
    it != external_client_sockets.end(); it++)
  {
    delete it->second;
  }
  for (map<int,AnonymousServerSocket*>::iterator it = client_connection_servers.begin();
    it != client_connection_servers.end(); it++)
  {
    delete it->second;
  }
}

void ExternalClients::start_listening(int portnum_base)
{
  client_connection_servers[portnum_base] = new AnonymousServerSocket(portnum_base + get_party_num());
  client_connection_servers[portnum_base]->init();
  cerr << "Start listening on thread " << this_thread::get_id() << endl;
  cerr << "Party " << get_party_num() << " is listening on port " << (portnum_base + get_party_num()) 
        << " for external client connections." << endl;
}

int ExternalClients::get_client_connection(int portnum_base)
{
  map<int,AnonymousServerSocket*>::iterator it = client_connection_servers.find(portnum_base);
  if (it == client_connection_servers.end())
  {
    cerr << "Thread " << this_thread::get_id() << " didn't find server." << endl; 
    return -1;
  }
  cerr << "Thread " << this_thread::get_id() << " found server." << endl; 
  int client_id, socket;
  socket = client_connection_servers[portnum_base]->get_connection_socket(client_id);
  if (ctx == 0)
    ctx = new ssl_ctx("P" + to_string(get_party_num()));
  external_client_sockets[client_id] = new ssl_socket(io_service, *ctx, socket,
      "C" + to_string(client_id), "P" + to_string(get_party_num()), false);
  cerr << "Party " << get_party_num() << " received external client connection from client id: " << dec << client_id << endl;
  return client_id;
}


int ExternalClients::get_party_num() 
{
  return party_num;
}

ssl_socket* ExternalClients::get_socket(int id)
{
  if (external_client_sockets.find(id) == external_client_sockets.end())
    throw runtime_error("external connection not found for id " + to_string(id));
  return external_client_sockets[id];
}
