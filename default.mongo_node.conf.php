<?php

/**
 * @file
 * Default settings file for Mongo Node. Edit directly or via UI.
 */

$settings = array(
  'mongo_profile' => array(
    'label' => 'Profile',
    'bundles' => array(
      'user_profile' => array(
        'label' => 'User profile',
      ),
    ),
    'fieldable' => TRUE,
  ),
  'mongo_article' => array(
    'label' => 'Article',
    'bundles' => array(
      'article' => array(
        'label' => 'Article',
        'description' => '',
      ),
      'page' => array(
        'label' => 'Basic page',
        'description' => '',
      ),
    ),
    'fieldable' => TRUE,
  ),
);
