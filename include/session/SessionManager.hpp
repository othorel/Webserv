// #ifndef SESSIONMANAGER_HPP
// # define SESSIONMANAGER_HPP

// # include <map>
// # include <string>
// # include "Session.hpp"

// class SessionManager {

// 	private:

// 		std::map<std::string, Session> _sessions;
// 		SessionManager();
// 		~SessionManager();
// 		SessionManager(const SessionManager&);
// 		SessionManager& operator=(const SessionManager&);

// 	public:


// 		static SessionManager& getInstance();
// 		bool hasSession(const std::string& sessionID);
// 		Session& getSession(const std::string& sessionID);
// 		void createSession(const std::string& sessionID);
// 		void deleteSession(const std::string& sessionID);
// };

// #endif