#include <scope/preview.h>

#include <unity/scopes/ColumnLayout.h>
#include <unity/scopes/PreviewWidget.h>
#include <unity/scopes/PreviewReply.h>
#include <unity/scopes/Result.h>
#include <unity/scopes/VariantBuilder.h>

#include <iostream>

namespace sc = unity::scopes;

using namespace std;
using namespace scope;

Preview::Preview(const sc::Result &result, const sc::ActionMetadata &metadata) :
    sc::PreviewQueryBase(result, metadata) {
}

void Preview::cancelled() {
}

void Preview::run(sc::PreviewReplyProxy const& reply) {
    sc::Result result = PreviewQueryBase::result();
    // Support three different column layouts
    sc::ColumnLayout layout1col(1); // layout2col(2), layout3col(3);

    // We define 3 different layouts, that will be used depending on the
    // device. The shell (view) will decide which layout fits best.
    // If, for instance, we are executing in a tablet probably the view will use
    // 2 or more columns.
    // Column layout definitions are optional.
    // However, we recommend that scopes define layouts for the best visual appearance.

    // Single column layout
    layout1col.add_column( { "header", "actions" });

    // Two column layout
    //layout2col.add_column( { "image" });
    //layout2col.add_column( { "header", "summary" });

    // Three cokumn layout
    //layout3col.add_column( { "image" });
    //layout3col.add_column( { "header", "summary" });
    //layout3col.add_column( { });

    // Register the layouts we just created
    reply->register_layout( { layout1col }); //, layout2col, layout3col });

    // Define the header section
    sc::PreviewWidget header("header", "header");
    // It has title and a subtitle properties
    header.add_attribute_mapping("title", "title");
    header.add_attribute_mapping("subtitle", "artist");

    // Define the image section
    // sc::PreviewWidget image("image", "image");
    // It has a single source property, mapped to the result's art property
    // image.add_attribute_mapping("source", "art");

    // Define the summary section
    // sc::PreviewWidget description("summary", "text");
    // It has a text property, mapped to the result's description property
    // description.add_attribute_mapping("text", "description");

    // Define the actions section
    sc::PreviewWidget w_actions("actions", "actions");

    // Actions are built using tuples with an id, a label and a URI
    sc::VariantBuilder builder;
    builder.add_tuple({
        {"id", sc::Variant("cpmments")},
        {"label", sc::Variant("Comments")},
        {"uri", result["comments"]}
    });
    builder.add_tuple({
        {"id", sc::Variant("open")},
        {"label", sc::Variant("Open")},
        {"uri", result["uri"]}
    });
    w_actions.add_attribute_value("actions", builder.end());
    // Push each of the sections
    reply->push( { header, w_actions});
}

