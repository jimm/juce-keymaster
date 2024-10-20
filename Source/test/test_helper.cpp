#include "test_helper.h"
#include "../km/keymaster.h"
#include "../km/storage.h"
#include "../km/utils.h"

KeyMaster *load_test_data(DeviceManager &dev_mgr, String data_file_path) {
  KeyMaster *old_km = KeyMaster_instance();

  if (data_file_path == "")
    data_file_path = "../../../../Source/test/testdata.kmst";
  if (!File::isAbsolutePath(data_file_path)) {
    data_file_path =
      File::addTrailingSeparator(File::getCurrentWorkingDirectory().getFullPathName())
      + data_file_path;
  }
  File file(data_file_path);

  Storage s(dev_mgr, file);
  KeyMaster *new_km = s.load();
  if (s.has_error()) {          // new_km == old_km, instance already reset
    String err_str("test data load error: ");
    err_str << s.error();
    DBG(err_str);
    return nullptr;
  }

  new_km->set_testing(true);
  if (old_km != nullptr) {
    old_km->stop();
    delete old_km;
  }
  return new_km;
}

bool mm_eq(const MidiMessage &a, const MidiMessage &b) {
  if (mm_equal(a, b))
    return true;

  int a_size = a.getRawDataSize();
  int b_size = b.getRawDataSize();
  DBG("a.size = " << a_size << ", b.size = " << b_size);

  auto a_data = a.getRawData();
  auto b_data = b.getRawData();
  for (int i = 0; i < a_size || i < b_size; ++i) {
    String str = "  ";
    if (i < a_size) str << a_data[i];
    str << "\t";
    if (i < b_size) str << b_data[i];
    DBG(str);
  }
  return false;
}
