#pragma once

typedef int DBObjID;

// Storage/accessors for database ids. These ids are only used during
// loading and saving of data by a Storage object. Do NOT rely on their
// values to persist across saves.
//
// Any classes that subclass DBObj should be added to Storage::assign_ids().
class DBObj {
public:
  DBObj(DBObjID i) : _id(i) {}
  virtual ~DBObj() {}

  inline DBObjID id() { return _id; }
  inline void set_id(DBObjID id) { _id = id; }

private:
  DBObjID _id;
};
