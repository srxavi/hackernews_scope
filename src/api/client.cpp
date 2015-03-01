#include <api/client.h>

#include <core/net/error.h>
#include <core/net/http/client.h>
#include <core/net/http/content_type.h>
#include <core/net/http/response.h>
#include <QVariantMap>
#include <iostream>
#include <QDebug>

namespace http = core::net::http;
namespace net = core::net;

using namespace api;
using namespace std;

Client::Client(Config::Ptr config) :
    config_(config), cancelled_(false) {
}


void Client::get(const net::Uri::Path &path,
                 const net::Uri::QueryParameters &parameters, QJsonDocument &root) {
    // Create a new HTTP client
    auto client = http::make_client();

    // Start building the request configuration
    http::Request::Configuration configuration;

    // Build the URI from its components
    net::Uri uri = net::make_uri(config_->apiroot, path, parameters);
    configuration.uri = client->uri_to_string(uri);
    // Give out a user agent string
    configuration.header.add("User-Agent", config_->user_agent);

    // Build a HTTP request object from our configuration
    auto request = client->head(configuration);

    try {
        // Synchronously make the HTTP request
        // We bind the cancellable callback to #progress_report
        auto response = request->execute(
                    bind(&Client::progress_report, this, placeholders::_1));

        // Check that we got a sensible HTTP status code
        if (response.status != http::Status::ok) {
            throw domain_error(response.body);
        }
        // Parse the JSON from the response
        root = QJsonDocument::fromJson(response.body.c_str());
    } catch (net::Error &) {
        qWarning() << "Error loading page";
    }
}

Client::ItemIdList Client::get_top_stories(const string &query) {
    // This is the method that we will call from the Query class.
    // It connects to an HTTP source and returns the results.


    // In this case we are going to retrieve JSON data.
    QJsonDocument root;

    // Build a URI and get the contents.
    // The fist parameter forms the path part of the URI.
    // The second parameter forms the CGI parameters.
    get({"topstories.json"}, {{"q", query}}, root);
    // e.g. http://api.openweathermap.org/data/2.5/weather?q=QUERY&units=metric

    ItemIdList result;
    QVariantList variant = root.toVariant().toList().mid(0,20);
    for (QVariant i: variant) {
        result.items.emplace_back(i.toUInt());
    }
    return result;
}

Client::Item Client::get_item(unsigned int id) {
    QJsonDocument item_json;
    QString item_number = QString::number(id);
    Item item;
    get({"item", item_number.toStdString() + ".json"}, {{}}, item_json);
    QVariantMap data = item_json.toVariant().toMap();
    //item.by = data["id"].toInt();
    if (data["type"] == "story") {
        item.id = data["id"].toUInt();
        item.title = data["title"].toString().toStdString();
        item.url = data["url"].toString().toStdString();
        if (item.url.empty())
                item.url = "htts://news.ycombinator.com/item?id=" + data["id"].toString().toStdString();
        item.by = data["by"].toString().toStdString();
        item.score = data["score"].toUInt();
        for (QVariant kid: data["kids"].toList())
            item.kids.emplace_back(kid.toUInt());
        item.time = data["time"].toULongLong();
    }
    return item;
}

Client::ItemRes Client::top_stories(const string& query) {
    // This is the method that we will call from the Query class.
    // It connects to an HTTP source and returns the results.


    // In this case we are going to retrieve JSON data.
    QJsonDocument root;

    // Build a URI and get the contents.
    // The fist parameter forms the path part of the URI.
    // The second parameter forms the CGI parameters.
    get({"topstories.json"}, {{"q", query}}, root);
    // e.g. http://api.openweathermap.org/data/2.5/weather?q=QUERY&units=metric

    ItemRes result;

    // Read out the top stories
    QVariantList variant = root.toVariant().toList().mid(0,20);
    for(QVariant i: variant) {
        QJsonDocument item_json;
        QString item_number = i.toString();
        Item item;
        get({"item", item_number.toStdString() + ".json"}, {{}}, item_json);
        QVariantMap data = item_json.toVariant().toMap();
        //item.by = data["id"].toInt();
        if (data["type"] == "story") {
            item.id = data["id"].toUInt();
            item.title = data["title"].toString().toStdString();
            item.url = data["url"].toString().toStdString();
            item.comments_url = "https://news.ycombinator.com/item?id=" + data["id"].toString().toStdString();
            if (item.url.empty())
                    item.url = item.comments_url;
            item.by = data["by"].toString().toStdString();
            item.score = data["score"].toUInt();
            for (QVariant kid: data["kids"].toList())
                item.kids.emplace_back(kid.toUInt());
            result.items.emplace_back(item);
            item.time = data["time"].toULongLong();
        }
    }
    return result;
}

http::Request::Progress::Next Client::progress_report(
        const http::Request::Progress&) {

    return cancelled_ ?
                http::Request::Progress::Next::abort_operation :
                http::Request::Progress::Next::continue_operation;
}

void Client::cancel() {
    cancelled_ = true;
}

Config::Ptr Client::config() {
    return config_;
}

