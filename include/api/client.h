#ifndef API_CLIENT_H_
#define API_CLIENT_H_

#include <api/config.h>

#include <atomic>
#include <deque>
#include <map>
#include <string>
#include <core/net/http/request.h>
#include <core/net/uri.h>

#include <QJsonDocument>

namespace api {

/**
 * Provide a nice way to access the HTTP API.
 *
 * We don't want our scope's code to be mixed together with HTTP and JSON handling.
 */
class Client {
public:


    /**
     * Hacker news Item
     */

    struct Item {
        unsigned int id;
        bool deleted;
        std::string type;
        std::string by;
        unsigned long long time;
        std::string text;
        bool dead;
        unsigned int parent;
        std::deque<unsigned int>  kids;
        std::string url;
        std::string original_url;
        std::string comments_url;
        unsigned int score;
        std::string title;
        std::deque<unsigned int> parts;
    };

    /**
     * Hacker News User
     */
    struct User {
        unsigned int id;
        unsigned long delay;
        unsigned long created;
        unsigned long karma;
        std::string about;
        std::deque<unsigned int> submitted;
    };

    /**
     * A list of items
     */
    typedef std::deque<Item> ItemList;

    struct ItemRes {
        ItemList items;
    };

    struct ItemIdList {
        std::deque<unsigned int> items;
    };

    Client(Config::Ptr config);

    virtual ~Client() = default;

    /**
     * Get the top Hacker news storires
     */
    virtual ItemRes top_stories(const std::string &query);

    virtual ItemIdList get_top_stories(const std::string &query);

    virtual Item get_item(unsigned int id);

    /**
     * Get the comments for an Item
     */
    //virtual ItemList comments(const Item);

    /**
     * Cancel any pending queries (this method can be called from a different thread)
     */
    virtual void cancel();

    virtual Config::Ptr config();

protected:
    void get(const core::net::Uri::Path &path,
             const core::net::Uri::QueryParameters &parameters,
             QJsonDocument &root);
    /**
     * Progress callback that allows the query to cancel pending HTTP requests.
     */
    core::net::http::Request::Progress::Next progress_report(
            const core::net::http::Request::Progress& progress);

    /**
     * Hang onto the configuration information
     */
    Config::Ptr config_;

    /**
     * Thread-safe cancelled flag
     */
    std::atomic<bool> cancelled_;
};

}

#endif // API_CLIENT_H_

