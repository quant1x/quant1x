from dataclasses import fields, is_dataclass


def apply_defaults(obj):
    """Apply defaults to dataclass instances and nested structures.

    Field-level defaults can be provided using field(metadata={"default": ...}).
    This function will set the metadata default when the current value is falsy (None, empty, zero).
    """
    if obj is None:
        return

    if is_dataclass(obj):
        for f in fields(obj):
            name = f.name
            val = getattr(obj, name)
            meta_default = None
            if f.metadata:
                meta_default = f.metadata.get("default")
            # decide falsy: None, empty string, empty list/dict, numeric zero
            is_falsy = val is None or (isinstance(val, (str, list, dict)) and len(val) == 0)
            if isinstance(val, (int, float)) and val == 0:
                is_falsy = True
            if is_falsy and meta_default is not None:
                try:
                    setattr(obj, name, meta_default)
                    val = meta_default
                except Exception:
                    pass
            # recurse
            apply_defaults(val)
        return

    if isinstance(obj, dict):
        for k, v in obj.items():
            apply_defaults(v)
        return

    if isinstance(obj, list):
        for it in obj:
            apply_defaults(it)
        return

    # other types: nothing to do
    return
