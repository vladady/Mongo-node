Mongo Node
----------


This module is supossed to be a performance boost replacement for the standard
Node in Drupal.

Advantages over node:
- performance: faster saving and loading
- no tables for entity types or bundles in MySQL (too many tables already)
- creation of new entity types and entity bundles
- similar entity item administration to the node (no coding required)

The base entity is without a base table, all the properties and fields attached
to the Mongo entities are stored directly in Mongo.

This leads to faster saving and loading of entities, and less tables in Drupal.
At this moment mongo nodes revisions are not supported.

For use cases where revisions are not needed the module is better than the
standard node because it doesn't save a revision for every item.

The advantage of using it is that you can create new entity types with multiple
entity bundles via interface or code configuration also.
You don't need a new module for every new entity.

Requirements
------------

The following modules are required:
- Entity API (http://drupal.org/project/entity)
- MongoDB (http://drupal.org/project/mongodb)
- MongoDB Field Storage (http://drupal.org/project/mongodb)

Installation
------------
Step 1) Rename default.mongo_node.conf.php to mongo_node.conf.php
Step 2) sudo chown www-data mongo_node.conf.php
Step 3) Done!

The default settings create two content types:
- Article (with 2 bundles: Article and Basic page)
- Profile with 1 bundle: User Profile

Change owner of mongo_node.conf.php only on Local Dev server!!
This way you can add, edit or delete entity types or bundles in a safe way!

This assures a proper development workflow:
- developers add/edit/delete mongo nodes (entities) easily on local/dev servers
- content admins can add and edit content on live server
