
OVERVIEW
--------

This module provides Flag integration for the Apache Solr module.

WHERE
-----

After enabling this module you'll find new search filters (also known as
"facets") on the **Site configuration › Apache Solr › Enabled filters** page.
You'll find one (at least one) facet for every flag defined on your system.
Enable the ones you want. By default the facets only show flagged items but
you can configure them (in their block settings) to show also items that are
specifically unflagged.

HOW IT WORKS
------------

First, let's look at how Solr sees a document:

    <doc>
      <field name="title">My parrot</field>
      <field name="body">My parrot speaks several languages ...</field>
      <field name="nid">406</field>
      ...
    </doc>

If a document is flagged (by at least one user), a new field is added:

    <doc>
      ... (fields shown previously omitted) ...

      <field name="bs_flag_bookmarks_flagged">true</field>

    </doc>

"bs" stands for "boolean, single" (see schema.xml). This is fine for
global flags, but for non-global flags this is not enough: Solr wouldn't
be able to give us all the documents flagged by the current user. For
this reason, when a document is flagged by a non-global flag, a
multi-valued field is also added to the document:

    <doc>
      ... (fields shown previously omitted) ...

      <field name="bs_flag_bookmarks_flagged">true</field>

      <field name="im_flag_bookmarks_uid">6</field>
      <field name="im_flag_bookmarks_uid">18</field>
      <field name="im_flag_bookmarks_uid">56</field>

    </doc>

"im" stands for "integer, multiple". The numbers shown above are the
UIDs of the users who flagged the item.

OBSERVATIONS
------------

1. You must re-index your whole site content after enabling this module for
   the above Solr fields to be generated.

2. Whenever a node is flagged (or unflagged), the Flag Solr module marks is
   as "needs re-indexing". On a typical Drupal installation this means that
   the node will be re-indexed on a future cron run (plus 2 minutes Solr
   commit delay).

   This means that several minutes have to pass till flagged items are seen
   in the search results.

3. Changing a flag's machine-name necessitates re-indexing the site's content
   because the flag's machine-name is embedded in the Solr field names.

