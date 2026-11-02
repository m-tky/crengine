#ifndef LVLOGICAL_CASCADE_H_INCLUDED
#define LVLOGICAL_CASCADE_H_INCLUDED

// Fork-only cascade scratch state in css_style_rec_t::flags. The upstream
// flags occupy bits 0 and 1; these bits are cleared before a style is cached.
// No global context or public stylesheet API extension is needed.
namespace ForkLogicalCascade {
    enum {
        MappedHorizontal = 0x04,
        MappedVertical = 0x08,
        ForceHorizontal = 0x10,
        ForceVertical = 0x20,
        OverrideMask = ForceHorizontal | ForceVertical,
        StateMask = MappedHorizontal | MappedVertical | OverrideMask,
    };

    template <typename Flags> bool useVertical(Flags &flags, bool vertical) {
        if (flags & OverrideMask)
            vertical = (flags & ForceVertical) != 0;
        flags |= vertical ? MappedVertical : MappedHorizontal;
        return vertical;
    }

    inline bool isReplay(int flags) {
        return (flags & OverrideMask) != 0;
    }

    inline bool needsReplay(int flags, bool final_vertical) {
        // Remember every mapping axis, not just the first/last writing-mode:
        // a declaration between two mode changes can leave a wrong side set.
        return (flags & (final_vertical ? MappedHorizontal : MappedVertical)) != 0;
    }

    inline int overrideFor(bool vertical) {
        return vertical ? ForceVertical : ForceHorizontal;
    }

    template <typename Flags> void clear(Flags &flags) {
        flags &= ~StateMask;
    }
}

#endif // LVLOGICAL_CASCADE_H_INCLUDED
