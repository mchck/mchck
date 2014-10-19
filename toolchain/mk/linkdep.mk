# magic make snippet to select all objects that are required to be linked

define _update_symvar
$(1):= $$(call uniq,${$(1)})
endef

define _clean_symvar
$(1):= $$(call uniq,$$(filter-out $$$$%,${$(1)}))
endef

define _update_symvars
$$(foreach _var,$${_syms_files},$$(eval $$(call _update_symvar,$${_var})))
endef

define objdeps
$(call uniq,$(foreach __obj,$(1),${_syms.${__obj}}))
endef

# from <https://stackoverflow.com/questions/16144115/makefile-remove-duplicate-words-without-sorting>
define uniq
$(if $1,$(call uniq,$(filter-out $(lastword $1),$1)) $(lastword $1))
endef

GENERATE.linkdep=	${_libdir}/scripts/linkdep -o $@ $<


${LIBDEPCACHE}/%.linkdep: ${LIBDEPCACHE}/%.o
	cd "${LIBDEPCACHE}" && ${GENERATE.linkdep} --no-path

%.linkdep: %.o
	${GENERATE.linkdep}

_libdeps=	$(addprefix ${LIBDEPCACHE}/,${_libobjs:.o=.linkdep})
REALCLEANFILES+=	${_libdeps}
_applinkdeps=	${OBJS:.o=.linkdep}
CLEANFILES+=	${_applinkdeps}
_linkdeps=	${_libdeps} ${_applinkdeps}


ifeq ($(call is-make-clean),)
-include ${_linkdeps}
endif

$(foreach _iter,${_syms_files},$(eval $(call _update_symvars)))
$(foreach _iter,${_syms_files},$(eval $(call _clean_symvar,${_iter})))

LINKOBJS=	$(call uniq,${OBJS} $(call objdeps,${FORCEOBJS}))
