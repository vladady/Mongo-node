<?php

/**
 * @file
 * Controller for special loading, saving and menu items
 */

class MongoNode extends Entity {
  /**
   * Overrides defaultLabel() to provide a better label based on title.
   */
  protected function defaultLabel() {
    return $this->entityInfo['bundles'][$this->bundle()]['label'];
  }

  /**
   * Overrides defaultUri() to provide different uris for entity types.
   */
  protected function defaultUri() {
    return array('path' => $this->entityType . '/' . $this->identifier());
  }
}

class MongoNodeController extends EntityApiController {

  /**
   * Constructor as a helper to the parent constructor.
   */
  public function __construct($entityType) {
    parent::__construct($entityType);
  }

  /**
   * Loads MongoNode entities directly from MongoDB.
   *
   * @param array $ids
   *   If $ids not provided loads all entities(SearchAPI)
   * @param array $conditions
   *   Conditions for loading entities (use EFQ instead)
   *
   * @return MongoNode
   *   Returns an array of loaded entities indexed by id
   */
  public function load($ids = array(), $conditions = array()) {
    $entities = array();

    // Return ids of all items (Search API requests this).
    if ($ids === FALSE) {
      $collection = mongodb_collection('fields_current', $this->entityType);
      $result = $collection->find(array('_type' => $this->entityType));

      foreach ($result as $row) {
        $entities[$row['_id']] = $row['_id'];
      }
    }
    else {
      $ids = array_map('intval', $ids);

      if(empty($ids)) {
        return $entities;
      }

      // Retrieve entities for specific ids.
      $collection = mongodb_collection('fields_current', $this->entityType);
      $result = $collection->find(array('_id' => array('$in' => $ids)));

      static $settings = array();
      $attach_fields = array('image');

      if (!isset($settings[$this->entityType])) {
        $entity_properties = entity_get_property_info($this->entityType);

        $settings[$this->entityType]['properties'] = array_keys($entity_properties['properties']);
        $settings[$this->entityType]['fields_info'] = field_info_field_by_ids();
        $settings[$this->entityType]['field_instances'] = field_info_instances($this->entityType);
      }

      $properties = $settings[$this->entityType]['properties'];
      $fields_info = $settings[$this->entityType]['fields_info'];
      $field_instances = $settings[$this->entityType]['field_instances'];

      foreach ($result as $row) {
        // Store properties.
        $class = $this->entityInfo['entity class'];
        $entities[$row['_id']] = new $class(array(), $this->entityType);

        foreach ($properties as $property_name) {
          if (isset($row[$property_name])) {
            $entities[$row['_id']]->$property_name = $row[$property_name];
          }
        }
        // Get instances.
        foreach ($field_instances[$row['_bundle']] as $field_name => &$field_info) {
          $field_values = array();
          $field_id = $field_info['field_id'];

          // Store field values.
          if (!empty($row[$field_name])) {
            if ($fields_info[$field_id]['cardinality'] == 1) {
              $language = isset($row[$field_name]['_language']) ? $row[$field_name]['_language'] : LANGUAGE_NONE;
              unset($row[$field_name]['_language']);
              $field_values[$language][0] = $row[$field_name];
            }
            else {
              foreach ($row[$field_name] as $delta => $column_values) {
                $language = isset($column_values['_language']) ? $column_values['_language'] : LANGUAGE_NONE;

                if(is_array($column_values) && isset($column_values['_language'])){
                  unset($column_values['_language']);
                }
                $field_values[$language][$delta] = $column_values;
              }
            }
          }

          $entities[$row['_id']]->{$field_name} = $field_values;
          // Some fields need to be attached.
          if(in_array($fields_info[$field_id]['type'], $attach_fields)){
            $options = array('field_id' => $field_id);
            field_attach_load($this->entityType, $entities, NULL, $options);
          }
        }
      }
    }

    return $entities;
 }

  /**
   * Saves a MongoNode entity directly in MongoDB.
   *
   * @param MongoNode $entity
   *   Entity object to save
   *
   * @return MongoNode
   *   Newly saved MongoNode entity
   */
  public function save($entity) {
    $entity_type = $this->entityType;
    $bundle = $entity->type;
    $entity->is_new = !isset($entity->mid) || empty($entity->mid);

    if (empty($entity->created)) {
      $entity->created = REQUEST_TIME;
    }

    $entity->changed = REQUEST_TIME;

    // TODO - test entities without bundle.

    if(!(isset($entity->fast_import) && $entity->fast_import)){
      field_attach_presave($entity_type, $entity);
      module_invoke_all('entity_presave', $entity, $this->entityType);
    }
    else {
      job_autotag_entity_presave($entity);
    }

    if ($entity->is_new) {
      $op = 'insert';

      // Get a unique id.
      $db = mongodb();
      $id = $db->command(
        array(
          "findandmodify" => $this->entityType . '_ids',
          "query" => array('type' => $this->entityType),
          "update" => array('$inc' => array("c" => 1)),
        )
      );

      if ($id['ok'] && isset($id['value']['c'])) {
        $entity->mid = intval($id['value']['c']);
      }
      else {
        $collection = $db->{$this->entityType . '_ids'};
        $collection->save(array('type' => $this->entityType, 'c' => 2));
        $entity->mid = 1;
      }
    }
    else {
      $op = 'update';
    }

    $function = 'field_attach_' . $op;
    //$function($entity_type, $entity);

    // Create a new object.
    $new_entity = new stdClass();
    $new_entity->_id = intval($entity->mid);
    $new_entity->_type = $this->entityType;
    $new_entity->_bundle = $entity->type;
    $new_entity->status = isset($entity->status) ? $entity->status : 1;
    $new_entity->_language = isset($entity->language) ? $entity->language : LANGUAGE_NONE;
    $new_entity->language = $new_entity->_language;

    //Add the properties of the entity to the new entity
    static $entity_properties = array();
    if(!isset($entity_properties) || empty($entity_properties)){
      $entity_properties = entity_get_property_info();
    }

    foreach ($entity_properties[$entity_type]['properties'] as $property_name => &$property_metadata) {
      if (isset($entity->$property_name)) {
        $new_entity->$property_name = _mongo_node_property_storage_value($property_metadata['type'], $entity->$property_name);
      }
    }

    // Retrieve entities for specific ids.
    static $fields_info = array();
    if(!isset($field_info) || empty($fields_info)){
      $fields_info = field_info_field_by_ids();
    }

    static $field_instances = array();
    if(!isset($field_instances) || empty($field_instances)) {
      $field_instances = field_info_instances();
    }

    // Get instances.
    foreach ($field_instances[$entity_type][$bundle] as $field_name => &$field_info) {
      $field_values = array();
      $field_id = $field_info['field_id'];
      $field = $fields_info[$field_id];

      // Store field values.
      if (!empty($entity->$field_name)) {
        foreach ($entity->$field_name as $language => $values) {
          /*if (empty($values[0])) {
            continue;
          }*/
          if ($field['cardinality'] == 1) {
            foreach (reset($values) as $column_name => $column_value) {
              $field_values[$column_name] = $column_value;
              if ($language != LANGUAGE_NONE) {
                $field_values['_language'][$column_name] = $column_value;
              }
            }
          }
          else {
            // Collapse deltas.
            $values = array_values($values);
            if ($field['cardinality'] > 1 && count($values) > $field['cardinality']) {
              throw new MongodbStorageException(
                t('Invalid delta for @field_name, not saving @entity_type @entity_id',
                  array(
                    '@field_name' => $field_name,
                    '@entity_type' => $entity_type,
                    '@entity_id' => $entity_id,
                  )
                )
              );
            }
            foreach ($values as $delta => $column_values) {
              $store_values = array();
              foreach ($column_values as $column_name => $column_value) {
                if (isset($field['columns'][$column_name])) {
                  $store_values[$column_name] = _mongo_node_field_storage_value($field['columns'][$column_name]['type'], $column_values[$column_name]);
                }
              }
              // Collapse the field structure.
              if ($language != LANGUAGE_NONE) {
                $store_values['_language'] = $language;
              }
              $field_values[] = $store_values;
            }
          }

        }
      }
      $new_entity->$field_name = empty($field_values) ? NULL : $field_values;
    }

    // Invoke field attach.

    if(!(isset($entity->fast_import) && $entity->fast_import)){
      module_invoke_all('entity_' . $op, $new_entity, $this->entityType);
    }
    else{
      if(module_exists('search_api')){
        $func = 'search_api_entity_' . $op;
        $func($new_entity, $this->entityType);
      }

      $func = 'job_general_entity_' . $op;
      if(function_exists($func)){
        $func($new_entity, $this->entityType);
      }
    }

    $is_new = isset($entity->is_new) ? TRUE : FALSE;
    unset($new_entity->is_new);

    // Save the object.
    try {
      $collection = mongodb_collection('fields_current', $this->entityType);
      $collection->save($new_entity);
    }
    catch (Exception $e){
      watchdog_exception('mongo_node', $e, 'Errro encounterd when saving');
    }

    // Return an object of the entity base class to operate upon.
    $class = $this->entityInfo['entity class'];
    $entity = new $class((array) $new_entity, $this->entityType);
    return $entity;
  }

  /**
   * Deletes MongoDB entities.
   *
   * @param array $ids
   *   An array of entity ids to delete
   */
  public function delete($ids = array()) {
    $collection = mongodb_collection('fields_current', $this->entityType);

    $entities = $ids ? $this->load($ids) : FALSE;

    if (!$entities) {
      return;
    }

    try {
      $ids = array_keys($entities);

      $collection->remove(array('mid' => array('$in' => $ids)));

      foreach ($entities as $id => $entity) {
        if (module_exists('search')) {
          search_reindex($id, $this->entityType);
        }
        field_attach_delete($this->entityType, $entity);
        module_invoke_all('entity_delete', $entity, $this->entityType);
      }
    }
    catch (Exception $e) {
      watchdog_exception($this->entityType, $e);
      throw $e;
    }
  }

}

class MongoNodeUIController extends EntityDefaultUIController {
  /**
   * Constructor as a helper to the parent constructor.
   */
  public function __construct($entity_type, $entity_info) {
    parent::__construct($entity_type, $entity_info);
  }

  /**
   * Overrides hook_menu() defaults.
   *
   * Main reason for doing this is that
   * parent class hook_menu() is optimized for entity type administration.
   */
  public function hook_menu() {
    $set = mongo_node_settings();

    $items[$this->path] = array(
      'title' => t('Manage @label', array('@label' => $set[$this->entityType]['label'])),
      'page callback' => 'drupal_get_form',
      'page arguments' => array('mongo_node_admin_content', 3),
      'access callback' => 'mongo_node_access',
      'access arguments' => array('view', $this->entityType),
      'file' => 'mongo_node.admin.inc',
      'file path' => drupal_get_path('module', 'mongo_node'),
      'type' => MENU_LOCAL_TASK,
    );

    $items[$this->path . '/add'] = array(
      'title' => 'Add entities',
      'description' => 'Add entities',
      'page callback' => 'mongo_node_add_page',
      'access callback' => 'mongo_node_access',
      'access arguments' => array('create', $this->entityType),
      'file' => 'mongo_node.admin.inc',
      'file path' => drupal_get_path('module', 'mongo_node'),
    );

    $set = mongo_node_settings();

    foreach ($set[$this->entityType]['bundles'] as $bundle => $settings) {
      $items[$this->path . '/add/' . $bundle] = array(
        'title' => 'Add ' . $settings['label'],
        'page callback' => 'mongo_node_add',
        'page arguments' => array($this->entityType, $bundle),
        'access callback' => 'mongo_node_access',
        'access arguments' => array('create', $bundle),
        'file' => 'mongo_node.admin.inc',
        'file path' => drupal_get_path('module', 'mongo_node'),
      );
    }

    $items[$this->entityType . '/%mongo_node'] = array(
      'title' => 'mongo entity',
      'page callback' => 'mongo_node_page_view',
      'page arguments' => array(0, 1),
      'access callback' => 'mongo_node_access',
      'access arguments' => array('view', 1),
      'load arguments' => array($this->entityType),
      'file' => 'mongo_node.admin.inc',
      'file path' => drupal_get_path('module', 'mongo_node'),
    );

    $items[$this->entityType . '/%mongo_node/view'] = array(
      'title' => 'View',
      'type' => MENU_DEFAULT_LOCAL_TASK,
      'weight' => -10,
    );

    $items[$this->entityType . '/%mongo_node/edit'] = array(
      'title' => 'Edit',
      'page callback' => 'mongo_node_page_edit',
      'page arguments' => array(0, 1),
      'access callback' => 'mongo_node_access',
      'access arguments' => array('update', 1),
      'file' => 'mongo_node.admin.inc',
      'file path' => drupal_get_path('module', 'mongo_node'),
      'type' => MENU_LOCAL_TASK,
      'weight' => 0,
    );

    $items[$this->entityType . '/%mongo_node/delete'] = array(
      'title' => 'Delete',
      'page callback' => 'drupal_get_form',
      'page arguments' => array('mongo_node_page_delete', 0, 1),
      'access callback' => 'mongo_node_access',
      'access arguments' => array('delete', 1),
      'file' => 'mongo_node.admin.inc',
      'file path' => drupal_get_path('module', 'mongo_node'),
      'type' => MENU_LOCAL_TASK,
      'weight' => 1,
    );

    return $items;
  }

}
