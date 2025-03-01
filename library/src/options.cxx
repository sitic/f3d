#include "options.h"

#include "log.h"

#include "vtkF3DConfigure.h"

#include <map>
#include <type_traits>
#include <variant>

namespace f3d
{
//----------------------------------------------------------------------------
class options::internals
{
public:
  using OptionVariant =
    std::variant<bool, int, double, std::string, std::vector<int>, std::vector<double> >;

  template<typename T, typename U>
  struct IsTypeValid;

  template<typename T, typename... Ts>
  struct IsTypeValid<T, std::variant<Ts...> > : public std::disjunction<std::is_same<T, Ts>...>
  {
  };

  template<typename T>
  void init(const std::string& name, const T& value)
  {
    static_assert(IsTypeValid<T, OptionVariant>::value);
    this->Options[name] = value;
  }

  template<typename T>
  void set(const std::string& name, const T& value)
  {
    static_assert(!std::is_array_v<T> && !std::is_pointer_v<T>);
    try
    {
      T& opt = std::get<T>(this->Options.at(name));
      opt = value;
    }
    catch (const std::bad_variant_access&)
    {
      log::error("Trying to set option ", name, " with incompatible type");
    }
    catch (const std::out_of_range&)
    {
      log::error("Options ", name, " does not exist");
    }
  }

  template<typename T>
  void get(const std::string& name, T& value) const
  {
    try
    {
      value = std::get<T>(this->Options.at(name));
    }
    catch (const std::bad_variant_access&)
    {
      log::error("Trying to get option ", name, " with incompatible type");
      return;
    }
    catch (const std::out_of_range&)
    {
      log::error("Options ", name, " does not exist");
      return;
    }
  }

  template<typename T>
  T get(const std::string& name) const
  {
    T val = {};
    this->get(name, val);
    return val;
  }

  template<typename T>
  T& getRef(const std::string& name)
  {
    try
    {
      return std::get<T>(this->Options.at(name));
    }
    catch (const std::bad_variant_access&)
    {
      std::string error = "Trying to set option " + name + " with incompatible type";
      log::error(error);
      throw options::exception(error + "\n");
    }
    catch (const std::out_of_range&)
    {
      std::string error = "Options " + name + " does not exist";
      log::error(error);
      throw options::exception(error + "\n");
    }
  }

  std::map<std::string, OptionVariant> Options;
};

//----------------------------------------------------------------------------
options::options()
  : Internals(new options::internals)
{
  // Loader/Loading
  this->Internals->init("animation-index", 0);
  this->Internals->init("geometry-only", false);
  this->Internals->init("progress", false);
  this->Internals->init("camera-index", -1);
  this->Internals->init("color", std::vector<double>{ 1., 1., 1. });
  this->Internals->init("emissive-factor", std::vector<double>{ 1., 1., 1. });
  this->Internals->init("line-width", 1.0);
  this->Internals->init("metallic", 0.0);
  this->Internals->init("normal-scale", 1.0);
  this->Internals->init("opacity", 1.0);
  this->Internals->init("point-size", 10.0);
  this->Internals->init("roughness", 0.3);
  this->Internals->init("texture-base-color", std::string());
  this->Internals->init("texture-emissive", std::string());
  this->Internals->init("texture-material", std::string());
  this->Internals->init("texture-normal", std::string());

  // Loading but should be checked ?
  this->Internals->init("up", std::string("+Y"));

  this->Internals->init("cells", false);
  this->Internals->init("component", -1);

  this->Internals->init("scalars", F3D_RESERVED_STRING);

  this->Internals->init("fullscreen", false);
  this->Internals->init("hdri", std::string());
  this->Internals->init("background-color", std::vector<double>{ 0.2, 0.2, 0.2 });
  this->Internals->init("font-file", std::string());

  // Rendering/Dynamic
  this->Internals->init("axis", false);
  this->Internals->init("bar", false);
  this->Internals->init("blur-background", false);
  this->Internals->init("camera-azimuth-angle", 0.0);
  this->Internals->init("camera-elevation-angle", 0.0);
  this->Internals->init("camera-focal-point", std::vector<double>());
  this->Internals->init("camera-position", std::vector<double>());
  this->Internals->init("camera-view-angle", 0.0);
  this->Internals->init("camera-view-up", std::vector<double>());
  this->Internals->init("colormap",
    std::vector<double>{
      0.0, 0.0, 0.0, 0.0, 0.4, 0.9, 0.0, 0.0, 0.8, 0.9, 0.9, 0.0, 1.0, 1.0, 1.0, 1.0 });
  this->Internals->init("denoise", false);
  this->Internals->init("depth-peeling", false);
  this->Internals->init("edges", false);
  this->Internals->init("filename", false);
  this->Internals->init("fps", false);
  this->Internals->init("fxaa", false);
  this->Internals->init("grid", false);
  this->Internals->init("inverse", false);
  this->Internals->init("metadata", false);
  this->Internals->init("point-sprites", false);
  this->Internals->init("range", std::vector<double>());
  this->Internals->init("raytracing", false);
  this->Internals->init("samples", 5);
  this->Internals->init("ssao", false);
  this->Internals->init("tone-mapping", false);
  this->Internals->init("trackball", false);
  this->Internals->init("volume", false);
};

//----------------------------------------------------------------------------
options::~options()
{
  delete this->Internals;
}

//----------------------------------------------------------------------------
options::options(const options& opt)
  : Internals(new options::internals)
{
  this->Internals->Options = opt.Internals->Options;
}

//----------------------------------------------------------------------------
options& options::operator=(const options& opt)
{
  this->Internals->Options = opt.Internals->Options;
  return *this;
}

//----------------------------------------------------------------------------
options::options(options&& other)
{
  this->Internals = other.Internals;
  other.Internals = nullptr;
}

//----------------------------------------------------------------------------
options& options::operator=(options&& other)
{
  if (this != &other)
  {
    delete this->Internals;
    this->Internals = other.Internals;
    other.Internals = nullptr;
  }
  return *this;
}

//----------------------------------------------------------------------------
options& options::set(const std::string& name, bool value)
{
  this->Internals->set(name, value);
  return *this;
}

//----------------------------------------------------------------------------
options& options::set(const std::string& name, int value)
{
  this->Internals->set(name, value);
  return *this;
}

//----------------------------------------------------------------------------
options& options::set(const std::string& name, double value)
{
  this->Internals->set(name, value);
  return *this;
}

//----------------------------------------------------------------------------
options& options::set(const std::string& name, const std::string& value)
{
  this->Internals->set(name, value);
  return *this;
}

//----------------------------------------------------------------------------
options& options::set(const std::string& name, const char* value)
{
  this->Internals->set(name, std::string(value));
  return *this;
}

//----------------------------------------------------------------------------
options& options::set(const std::string& name, const std::vector<int>& values)
{
  this->Internals->set(name, values);
  return *this;
}

//----------------------------------------------------------------------------
options& options::set(const std::string& name, const std::vector<double>& values)
{
  this->Internals->set(name, values);
  return *this;
}

//----------------------------------------------------------------------------
options& options::set(const std::string& name, std::initializer_list<int> values)
{
  this->Internals->set(name, std::vector<int>(values));
  return *this;
}

//----------------------------------------------------------------------------
options& options::set(const std::string& name, std::initializer_list<double> values)
{
  this->Internals->set(name, std::vector<double>(values));
  return *this;
}

//----------------------------------------------------------------------------
void options::get(const std::string& name, bool& value) const
{
  this->Internals->get(name, value);
}

//----------------------------------------------------------------------------
void options::get(const std::string& name, int& value) const
{
  this->Internals->get(name, value);
}

//----------------------------------------------------------------------------
void options::get(const std::string& name, double& value) const
{
  this->Internals->get(name, value);
}

//----------------------------------------------------------------------------
void options::get(const std::string& name, std::string& value) const
{
  this->Internals->get(name, value);
}

//----------------------------------------------------------------------------
void options::get(const std::string& name, std::vector<int>& value) const
{
  this->Internals->get(name, value);
}

//----------------------------------------------------------------------------
void options::get(const std::string& name, std::vector<double>& value) const
{
  this->Internals->get(name, value);
}

//----------------------------------------------------------------------------
bool options::getAsBool(const std::string& name) const
{
  return this->Internals->get<bool>(name);
}

//----------------------------------------------------------------------------
int options::getAsInt(const std::string& name) const
{
  return this->Internals->get<int>(name);
}

//----------------------------------------------------------------------------
double options::getAsDouble(const std::string& name) const
{
  return this->Internals->get<double>(name);
}

//----------------------------------------------------------------------------
std::string options::getAsString(const std::string& name) const
{
  return this->Internals->get<std::string>(name);
}

//----------------------------------------------------------------------------
std::vector<int> options::getAsIntVector(const std::string& name) const
{
  return this->Internals->get<std::vector<int> >(name);
}

//----------------------------------------------------------------------------
std::vector<double> options::getAsDoubleVector(const std::string& name) const
{
  return this->Internals->get<std::vector<double> >(name);
}

//----------------------------------------------------------------------------
bool& options::getAsBoolRef(const std::string& name)
{
  return this->Internals->getRef<bool>(name);
}

//----------------------------------------------------------------------------
int& options::getAsIntRef(const std::string& name)
{
  return this->Internals->getRef<int>(name);
}

//----------------------------------------------------------------------------
double& options::getAsDoubleRef(const std::string& name)
{
  return this->Internals->getRef<double>(name);
}

//----------------------------------------------------------------------------
std::string& options::getAsStringRef(const std::string& name)
{
  return this->Internals->getRef<std::string>(name);
}

//----------------------------------------------------------------------------
std::vector<int>& options::getAsIntVectorRef(const std::string& name)
{
  return this->Internals->getRef<std::vector<int> >(name);
}

//----------------------------------------------------------------------------
std::vector<double>& options::getAsDoubleVectorRef(const std::string& name)
{
  return this->Internals->getRef<std::vector<double> >(name);
}

}
