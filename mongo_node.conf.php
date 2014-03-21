<?php $settings = array (
  'mongo_job' =>
  array (
    'label' => t('Job'),
    'bundles' =>
    array (
      'job_per_link' =>
      array (
        'label' => t('Imported jobs'),
        'description' => '',
        'view modes' => array (
          'full' => array (
            'label' => 'Full content',
            'custom settings' => FALSE,
          ),
          'teaser' => array (
            'label' => 'Teaser',
            'custom settings' => TRUE
          ),
          'email' => array (
            'label' => 'Email',
            'custom settings' => TRUE
          ),
        ),
      ),
      'job_per_template' =>
      array (
        'label' => t('Job per template'),
        'description' => '',
        'view modes' => array (
          'full' => array (
            'label' => 'Full content',
            'custom settings' => FALSE,
          ),
          'teaser' => array (
            'label' => 'Teaser',
            'custom settings' => TRUE
          ),
          'email' => array (
            'label' => 'Email',
            'custom settings' => TRUE
          ),
        ),
      ),
    ),
   'properties' => array(
      'mid' => array(
        'label' => t('Entity ID'),
        'type' => 'integer',
        'description' => t('The primary identifier for a server.'),
      ),
      'type' => array(
        'label' => t('Type'),
        'type' => 'text',
        'description' => t('The type of the entity.'),
      ),
      'language' => array(
        'label' => t('Language'),
        'type' => 'text',
        'description' => t('The language of the entity.'),
      ),
      'title' => array(
        'label' => t('Title'),
        'type' => 'text',
        'description' => t('The title of the entity.'),
        'setter callback' => 'entity_property_verbatim_set',
      ),
      'uid' => array(
        'label' => t('Uid'),
        'type' => 'integer',
        'description' => t('The user id.'),
      ),
      'status' => array(
        'label' => t('Status'),
        'type' => 'boolean',
        'description' => t('The status of the entity.'),
      ),
      'created' => array(
        'label' => t('Date created'),
        'type' => 'date',
        'description' => t('The creation date.'),
      ),
      'changed' => array(
        'label' => t('Date changed'),
        'type' => 'date',
        'description' => t('The date the item was changed.'),
      ),
      'sticky' => array(
        'label' => t('Sticky in lists'),
        'type' => 'integer',
        'description' => t('Show as sticky in lists.'),
      ),
      'imported_type' => array(
        'label' => t('Type of imported item'),
        'type' => 'text',
      ),
      'fast_import' => array(
        'label' => t('Fast import'),
        'type' => 'boolean',
      ),
    ),
    'fieldable' => true,
  ),
  'mongo_article' =>
  array (
    'label' => t('Article'),
    'bundles' =>
    array (
      'article_per_link' =>
      array (
        'label' => t('Article per link'),
        'description' => '',
        'view modes' => array (
          'full' => array (
            'label' => 'Full content',
            'custom settings' => FALSE,
          ),
          'teaser' => array (
            'label' => 'Teaser',
            'custom settings' => TRUE
          ),
        ),
      ),
      'article_per_description' =>
      array (
        'label' => t('Article per template'),
        'description' => '',
        'view modes' => array (
          'full' => array (
            'label' => 'Full content',
            'custom settings' => FALSE,
          ),
          'teaser' => array (
            'label' => 'Teaser',
            'custom settings' => TRUE
          ),
        ),
      ),
    ),
    'fieldable' => true,
  ),
  'company' =>
  array (
    'label' => t('Company'),
    'bundles' =>
    array (
      'company' =>
      array (
        'label' => t('Company'),
        'description' => '',
        'view modes' => array (
          'full' => array (
            'label' => 'Full content',
            'custom settings' => FALSE,
          ),
          'teaser' => array (
            'label' => 'Teaser',
            'custom settings' => TRUE
          ),
          'profile' => array (
            'label' => 'Profile',
            'custom settings' => TRUE
          ),
          'company_list' => array (
            'label' => 'Company list',
            'custom settings' => TRUE
          ),
          'email' => array (
            'label' => 'Email',
            'custom settings' => TRUE
          ),
        ),
      ),
    ),
    'properties' => array(
      'mid' => array(
        'label' => t('Entity ID'),
        'type' => 'integer',
        'description' => t('The primary identifier for a server.'),
      ),
      'type' => array(
        'label' => t('Type'),
        'type' => 'text',
        'description' => t('The type of the entity.'),
      ),
      'language' => array(
        'label' => t('Language'),
        'type' => 'text',
        'description' => t('The language of the entity.'),
      ),
      'title' => array(
        'label' => t('Company name'),
        'type' => 'text',
        'description' => t('The title of the entity.'),
        'setter callback' => 'entity_property_verbatim_set',
      ),
      'uid' => array(
        'label' => t('Uid'),
        'type' => 'integer',
        'description' => t('The user id.'),
      ),
      'status' => array(
        'label' => t('Status'),
        'type' => 'boolean',
        'description' => t('The status of the entity.'),
      ),
      'created' => array(
        'label' => t('Date created'),
        'type' => 'date',
        'description' => t('The creation date.'),
      ),
      'changed' => array(
        'label' => t('Date changed'),
        'type' => 'date',
        'description' => t('The date the item was changed.'),
      ),
      'sticky' => array(
        'label' => t('Sticky in lists'),
        'type' => 'integer',
        'description' => t('Show as sticky in lists.'),
      ),
      'crawled_source' => array(
        'label' => t('Source for crawled jobs'),
        'type' => 'text',
      ),
      'imported_type' => array(
        'label' => t('Source for crawled jobs'),
        'type' => 'text',
      ),
    ),
    'fieldable' => true,
  ),
  'cv' =>
  array (
    'label' => t('CV'),
    'bundles' =>
    array (
      'cv' =>
      array (
        'label' => t('CV'),
        'description' => '',
        'view modes' => array (
          'full' => array (
            'label' => 'Full content',
            'custom settings' => FALSE,
          ),
          'teaser' => array (
            'label' => 'Teaser',
            'custom settings' => TRUE
          ),
          'profile' => array (
            'label' => 'Profile',
            'custom settings' => TRUE
          ),
          'email_view' => array (
            'label' => 'Email',
            'custom settings' => TRUE
          ),
        ),
      ),
    ),
    'fieldable' => true,
  ),
);
