var nwNatives = requireNative('nw_natives');
var SQLiteVersion;

apiBridge.registerCustomHook(function(bindingsAPI) {
  var apiFunctions = bindingsAPI.apiFunctions;
  
  apiFunctions.setHandleRequest('lastID', function() {
      return bindingUtil.sendRequestSync('nw.SQLite.lastID', $Array.from(arguments), undefined, undefined)[0];
  });

  apiFunctions.setHandleRequest('getVersion', function () {
    return bindingUtil.sendRequestSync('nw.SQLite.getVersion', [], undefined, undefined)[0];
  });

  bindingsAPI.compiledApi.__defineGetter__('version', function () {
    if (!SQLiteVersion)
        SQLiteVersion = nw.SQLite.getVersion();
    return SQLiteVersion;
  });

});

//exports.binding = nw_binding.generate();
