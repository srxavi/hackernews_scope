#include <boost/algorithm/string/trim.hpp>

#include <scope/localization.h>
#include <scope/query.h>

#include <unity/scopes/Annotation.h>
#include <unity/scopes/CategorisedResult.h>
#include <unity/scopes/CategoryRenderer.h>
#include <unity/scopes/QueryBase.h>
#include <unity/scopes/SearchReply.h>

#include <iomanip>
#include <sstream>
#include <QDebug>

namespace sc = unity::scopes;
namespace alg = boost::algorithm;

using namespace std;
using namespace api;
using namespace scope;


/** 
 * Define the layout for the stories results
 * 
 * There are four fields: title, author, url and score.
 * 
 */

const static string STORY_TEMPLATE = 
        R"(
{
        "schema-version": 1,
        "template": {
        "category-layout": "grid",
        "card-layout": "horizontal",
        "card-size": "small"
        },
        "components": {
            "title": "title",
            "subtitle": "author"
            }
        }
        )";

Query::Query(const sc::CannedQuery &query, const sc::SearchMetadata &metadata,
             Config::Ptr config) :
    sc::SearchQueryBase(query, metadata), client_(config) {
}

void Query::cancelled() {
    client_.cancel();
}


void Query::run(sc::SearchReplyProxy const& reply) {
    try {
        // Start by getting information about the query
        const sc::CannedQuery &query(sc::SearchQueryBase::query());

        // Trim the query string of whitespace
        string query_string = alg::trim_copy(query.query_string());

        // the Client is the helper class that provides the results
        // without mixing APIs and scopes code.
        // Add your code to retreive xml, json, or any other kind of result
        // in the client.
        Client::ItemIdList item_res;
        
        auto items_cat = reply->register_category("stories", "Top Stories", "", query, sc::CategoryRenderer(STORY_TEMPLATE));
        item_res = client_.get_top_stories(query_string);

        for (unsigned int item_id: item_res.items) {
            sc::CategorisedResult res(items_cat);
            Client::Item item = client_.get_item(item_id);

            stringstream ss(stringstream::in | stringstream::out);

            ss << QString::number(item.score).toStdString() << " points by " << item.by << " | " << QString::number(item.kids.size()).toStdString() << " comments";

            res.set_uri(item.url);
            res.set_title(item.title);

            res["author"] = ss.str();
            res["comments"] = item.comments_url;
            // Push the result
            if (!reply->push(res)) {
                // If we fail to push, it means the query has been cancelled.
                // So don't continue;
                return;
            }
        }

    } catch (domain_error &e) {
        // Handle exceptions being thrown by the client API
        cerr << e.what() << endl;
        reply->error(current_exception());
    } catch (exception &e){
        qWarning() << e.what();
    }
}

