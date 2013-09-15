# magic make snippet to select all objects that are required to be linked

define _update_symvar
$(1):= $$(sort ${$(1)})
endef

define _clean_symvar
$(1):= $$(sort $$(filter-out $$$$%,${$(1)}))
endef

define _update_symvars
$$(foreach _var,$${_syms_files},$$(eval $$(call _update_symvar,$${_var})))
endef

define objdeps
$(sort $(foreach __obj,$(1),${_syms.${__obj}}))
endef

# ${_libdir}/cache/%.linkdep: ${_libdir}/cache/%.o
# 	${_libdir}/scripts/linkdep -o $@ $<

GENERATE_LINKDEP=	${_libdir}/scripts/linkdep -o $@ $<


LIBDEPCACHE=	${_libdir}/cache

# ${LIBDEPCACHE}/%.linkdep: ${LIBDEPCACHE}/%.o
# 	${GENERATE_LINKDEP}

%.linkdep: %.o
	${GENERATE_LINKDEP}

_libdeps=	$(addprefix ${LIBDEPCACHE}/,${_libobjs:.o=.linkdep})
REALCLEANFILES+=	${_libdeps}
_applinkdeps=	${OBJS:.o=.linkdep}
CLEANFILES+=	${_applinkdeps}
_linkdeps=	${_libdeps} ${_applinkdeps}


ifneq (${MAKECMDGOALS},clean)
-include ${_linkdeps}
endif

$(foreach _iter,${_syms_files},$(eval $(call _update_symvars)))
$(foreach _iter,${_syms_files},$(eval $(call _clean_symvar,${_iter})))

LINKOBJS=	$(call objdeps,${FORCEOBJS})
