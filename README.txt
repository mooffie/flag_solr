
OVERVIEW
--------

This module provides Flag integration for the Apache Solr module.


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

