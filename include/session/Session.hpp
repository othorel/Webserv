#ifndef SESSION_HPP
# define SESSION_HPP

# include <map>
# include <string>

class Session {

    private:

        std::map<std::string, std::string> _data;

    public:

        Session();
        ~Session();

        void setSession(const std::string& key, const std::string& value);
        std::string getSession(const std::string& key) const;
        bool hasSession(const std::string& key) const;
        void removeSession(const std::string& key);

};

#endif