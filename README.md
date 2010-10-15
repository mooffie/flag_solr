
Overview
--------

This module provides Apache Solr integration for the [Flag][1] module.

Requirements
------------

This module has been developed against the **2.x branch** of Drupal's [Apache
Solr][2] module. It hasn't been tested, and probably doesn't work, with the 1.x
branch.

Status
------

This module has just been started (2010-10-14) so I consider it in **alpha
stage**. Any reports on its use would be greatly appreciated.

Where
-----

After enabling this module you'll find new search filters (also known as
"facets") on the **Site configuration › Apache Solr › Enabled filters** page.
You'll find one (at least one) facet for every flag defined on your system.
Enable the ones you want. By default the facets only show flagged items but
you can configure them (in their block settings) to also show items that are
specifically unflagged.

How it works
------------

It could be useful for the site administrator to understand how this module
works.

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
UIDs of all the users who flagged the item.

Observations
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


[1]: http://drupal.org/project/flag
[2]: http://drupal.org/project/apachesolr
