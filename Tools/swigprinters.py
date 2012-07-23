import gdb
import gdb.types
import itertools
import re

log_file = None

def print_(msg):
    global log_file;
    
    if False:
      if log_file == None:
        log_file = open('~/swig_gdb.log', 'w')
      log_file.write(msg)

    print(msg)
    
class SwigStringPrinter:
  """
    Pretty print Swig String* types.
  """

  def __init__ (self, typename, val):
    self.typename = typename
    self.val = val
    
    try:
      self.t_swigstr_ptr = gdb.lookup_type("struct String").pointer()
      self.t_doh_base_ptr = gdb.lookup_type("DohBase").pointer()
    except Exception as err:
      print_("SwigStringPrinter: Could not retrieve gdb types.\n  %s.\n"%(str(err)))

  def display_hint(self):
    return 'string'

  def to_string(self):
    ret = "<invalid>"
    
    try:
      # Conversion taken from Swig Internals manual:
      # http://peregrine.hpc.uwm.edu/Head-node-docs/swig/2.0.4/Devel/internals.html#7
      # (*(struct String *)(((DohBase *)s)->data)).str
      dohbase = self.val.reinterpret_cast(self.t_doh_base_ptr).dereference()
      s = dohbase['data'].reinterpret_cast(self.t_swigstr_ptr).dereference()
      char_ptr = s['str']
      #if char_ptr.is_lazy is True:
      #  char_ptr.fetch_lazy ()
      ret = char_ptr.string()

    except Exception as exc:
      pass
    
    return ret

class SwigHashIterator:
    def __init__(self, val):

      try:
        self.t_doh_base_ptr = gdb.lookup_type("DohBase").pointer()

        self.val = val.reinterpret_cast(self.t_doh_base_ptr)
        self.valid = False
        self.hashtable = None
        self.hashsize = None
        self.nitems = None
        self.next_ = 0
        self.address = 0
        self.pos = 0;
        
        self.t_struct_hash_ptr = gdb.lookup_type("struct Hash").pointer()
        self.t_string_ptr = gdb.lookup_type("String").pointer()
        self.t_node_ptr = gdb.lookup_type("Node").pointer()
        self.t_hash_ptr = gdb.lookup_type("Hash").pointer()
        self.t_file_ptr = gdb.lookup_type("File").pointer()

        self.valid = self.sanity_check()
        self.address = self.val.dereference().address

      except Exception as err:
        print_("SwigHashIterator: Construction failed.\n %s.\n"%(str(err)))
    
    def __iter__(self):
      return self

    def next(self):

      if not self.valid:
        self.stop()
            
      while self.pos < self.hashsize:

        node_ptr = None
        try:
          if self.next_ == 0:
            node_ptr = self.hashtable[self.pos]
            self.pos = self.pos + 1
            if node_ptr == 0:
              continue
          else:
            node_ptr = self.next_
            self.next_ = 0

        except Exception as err:
          print_("SwigHashIterator(%s): Exception during proceeding to next:\n %s" % (str(self.address), str(err)) )
          self.stop();

        node = None
        key = None
        key_str = None
        obj = None
        
        try:
          node = node_ptr.dereference()
          
        except Exception as err:
          print_("SwigHashIterator(%s): Exception during dereferencing next:\n %s" % (str(self.address), str(err)) )
          #self.stop()
          continue

        try:
          self.next_ = node['next']
          key = node['key']
          obj = node['object']
          
          if key == 0:
            continue

        except Exception as err:
          print_("SwigHashIterator(%s): Exception during accessing hash node fields:\n %s" % (str(self.address), str(err)) )
          #self.stop()
          continue

        try:
          string_printer = SwigStringPrinter("String *", key)
          key_str = string_printer.to_string()
          
        except Exception as err:
          print_("SwigHashIterator(%s): Exception during extracting key string:\n %s" % (str(self.address), str(err)) )
          #self.stop()
          continue

        try:
          obj = self.cast_doh(obj)
          
        except Exception as err:
          print_("SwigHashIterator(%s): Exception during casting of value doh:\n %s" % (str(self.address), str(err)) )
          #self.stop()
          continue

        return (key_str, obj)

      self.stop()

    def cast_doh(self, doh):
              
      if doh == 0:
        return doh
        
      val_base = doh.reinterpret_cast(self.t_doh_base_ptr).dereference()
      val_type = val_base['type'].dereference()
      val_typestr = val_type['objname'].string()

      if "String" == val_typestr:
        doh = doh.reinterpret_cast(self.t_string_ptr)
      elif "File" == val_typestr:
        doh = doh.reinterpret_cast(self.t_file_ptr)
      # BUG: GDB Pyhton can not handle cyclic references yet
      #      so these casts are deactivated
      #elif "Hash" == val_typestr:
      #  doh = doh.reinterpret_cast(self.t_hash_ptr)
      #elif "Node" == val_typestr:
      #  doh = doh.reinterpret_cast(self.t_node_ptr)

      return doh
      
    def sanity_check(self):
      try:
        doh_base = self.val.dereference()
        hash_ = doh_base['data'].reinterpret_cast(self.t_struct_hash_ptr).dereference()
        self.hashtable = hash_['hashtable']
        self.hashsize = int(hash_['hashsize'])
        self.nitems = int(hash_['nitems'])
      except:
        return False
        
      return True
    
    def stop(self):
      raise StopIteration      

class NopIterator:
  
  def __init__(self):
    pass
  
  def __iter__(self):
    return self
    
  def next(self):
    raise StopIteration

class SwigHashPrinter:
  """
    Pretty print Swig Hash* types (also Node*).
  """
    
  def __init__ (self, typename, val):
    
    self.typename = typename
    self.val = val
    it = SwigHashIterator(val)
    self.valid = it.valid
    self.address = it.address
     
  def display_hint(self):
    return 'map'

  def to_string(self):
    return str(self.typename)
        
  def children(self):
    
    if not self.valid:
      print_("SwigHashPrinter: Invalid state.\n")
      return NopIterator()
    
    try:
      it = SwigHashIterator(self.val)
      return it
    except Exception as err:
      print_("SwigHashPrinter: Error during creation of children iterator. \n %s \n" %(str(err)))
      raise err

class RxPrinter(object):
    def __init__(self, name, function):
        super(RxPrinter, self).__init__()
        self.name = name
        self.function = function
        self.enabled = True

    def invoke(self, value):
        if not self.enabled:
            return None
        return self.function(self.name, value)

# A pretty-printer that conforms to the "PrettyPrinter" protocol from
# gdb.printing.  It can also be used directly as an old-style printer.
class Printer(object):
    def __init__(self, name):
        super(Printer, self).__init__()
        self.name = name
        self.subprinters = []
        self.lookup = {}
        self.enabled = True
        self.compiled_rx = re.compile('^([a-zA-Z0-9_: *]+)$')

    def add(self, name, function):        
        if not self.compiled_rx.match(name):
            raise ValueError, 'error: "%s" does not match' % name
        
        printer = RxPrinter(name, function)
        self.subprinters.append(printer)
        self.lookup[name] = printer
        print('Added pretty printer for %s. ' % (name))

    def __call__(self, val):
        typename = str(val.type)
        if typename in self.lookup:
            ret = self.lookup[typename].invoke(val)
            return ret

        # Cannot find a pretty printer.  Return None.
        return None

swig_printer = None

def register_swig_printers(obj):
    global swig_printer

    if obj is None:
      obj = gdb

    obj.pretty_printers.append(swig_printer)

def build_swig_printer():
    global swig_printer

    swig_printer = Printer("swig")
    swig_printer.add('String *', SwigStringPrinter)
    swig_printer.add('Hash *', SwigHashPrinter)
    swig_printer.add('Node *', SwigHashPrinter)
    print_("Loaded swig printers\n");

build_swig_printer()
