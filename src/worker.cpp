#include "master.h"
#include "worker.h"
#include "connection.h"
#include "plugin.h"

#include <stdlib.h>
#include <iostream>
#include <dlfcn.h>

Worker::Worker(const std::string &ip,unsigned short port)
    :listener(ip,port)
{
	master = nullptr;
	w_base = nullptr;
	w_exit_event = nullptr;
	w_plugins = nullptr;
	w_plugin_cnt = 0;

	if(!(SetupPlugins() && LoadPlugins())){
 
		std::cerr << "Worker:SetupPlugins() && LoadPlugins()" << std::endl;
	}
}

Worker::~Worker(){

	//需要在释放con之前解放插件，否则con释放时插件可能还在使用con的数据
	UnloadPlugins();
	//Master不进入run()函数，因此不对w_exit_event初始化
	if(w_exit_event)
		event_free(w_exit_event);

	if(w_base){
		ConnectionMap::iterator con_iter = con_map.begin();
		while(con_iter!=con_map.end()){
			Connection *con = con_iter->second;
			delete con;
			con_iter++;
		}
		event_base_free(w_base);
	}
	std::cout << "Worker Closed!" << std::endl;

	//移除插件需要在释放con之后，此时con中插件的数据已经被清理掉
	RemovePlugins();
}

void Worker::WorkerExitSignal(evutil_socket_t signo,short event,void* arg){

	event_base_loopexit((struct event_base*)arg,NULL);
}

bool Worker::Init(Master *master){

	this->master = master;

	if(!listener.InitListener(this)){

		std::cerr << "Worker:Listener::InitListener()" << std::endl;
		return false;
	}

	if(!(SetupPlugins() && LoadPlugins())){
 
		std::cerr << "Worker:SetupPlugins() && LoadPlugins()" << std::endl;
		return false;
	}

	return true;
}

void Worker::Run(){
	
	w_base = event_base_new();
	listener.AddListenEvent();
	w_exit_event = evsignal_new(w_base,SIGINT,Worker::WorkerExitSignal,w_base);
	evsignal_add(w_exit_event,NULL);

	event_base_dispatch(w_base);
	return ;
}

bool Worker::SetupPlugins(){

	std::string path;

	//for(int i=0;i< master->conf_para.PluginList.size();i++){
	  for(int i=0;i<1;i++){
		//path = master->conf_para.PluginList[i];
		path = "plugin/plugin_static/plugin_static.so";

		void *so = dlopen(path.c_str(),RTLD_LAZY);
		if(!so){

			std::cerr << dlerror() << std::endl;
			return false;
		}

		Plugin::SetupPlugin setup_plugin = (Plugin::SetupPlugin)dlsym(so,"SetupPlugin");
		Plugin::RemovePlugin remove_plugin = (Plugin::RemovePlugin)dlsym(so,"RemovePlugin");
		if(!setup_plugin || !remove_plugin){
			std::cerr << dlerror() << std::endl;
			dlclose(so);
			return false;
		}

		Plugin *plugin = setup_plugin();
		if(!plugin){
			dlclose(so);
			return false;
		}

		plugin->setup_plugin = setup_plugin;
		plugin->remove_plugin = remove_plugin;
		plugin->plugin_so = so;
		plugin->plugin_index = i;

		w_plugins = static_cast<Plugin* *>(realloc(w_plugins,sizeof(*w_plugins)*(w_plugin_cnt+1)));//扩大空间
		w_plugins[w_plugin_cnt++] = plugin;

	}

	return true;
}

void Worker::RemovePlugins(){

	Plugin *plugin;

	for(int i=0;i<w_plugin_cnt;i++){
		plugin = w_plugins[i];
		Plugin::RemovePlugin remove_plugin = plugin->remove_plugin;
		void *so = plugin->plugin_so;
		remove_plugin(plugin);
		dlclose(so);
	}

	free(w_plugins);
}

bool Worker::LoadPlugins(){

	Plugin *plugin;

	for(int i=0;i<w_plugin_cnt;i++){
		plugin = w_plugins[i];
		if(plugin->LoadPlugin(this,i)){
			plugin->plugin_is_loaded = true;
		}
		else{
			std::cerr << "Worker:Plugin::LoadPlugins()" <<std::endl;
			return false;
		}
	}

	return true;
}

void Worker::UnloadPlugins(){

	Plugin *plugin;

	for(int i=0;i<w_plugin_cnt;i++){

		plugin = w_plugins[i];
		if(plugin->plugin_is_loaded){
			plugin->FreePlugin(this,i);
		}
	}
}