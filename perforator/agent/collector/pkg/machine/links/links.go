package links

import (
	"errors"
	"fmt"

	"github.com/cilium/ebpf"

	"github.com/yandex/perforator/library/go/core/log"
	"github.com/yandex/perforator/perforator/pkg/linux/kallsyms"
)

type Links struct {
	l log.Logger

	tracepoints []*tracepoint
	kprobes     []*kprobe
}

type idempotentCloser interface {
	close() error
}

func closeLinks[T idempotentCloser](links []T) error {
	for _, link := range links {
		err := link.close()
		if err != nil {
			return err
		}
	}

	return nil
}

func (l *Links) Close() error {
	return errors.Join(
		closeLinks(l.tracepoints),
		closeLinks(l.kprobes),
	)
}

func NewLinks(l log.Logger) *Links {
	return &Links{
		l: l,
	}
}

func (l *Links) Tracepoint(group, name string, prog *ebpf.Program) error {
	tracepoint := &tracepoint{config: tracepointConfig{group: group, name: name}}
	err := tracepoint.create(prog)
	if err != nil {
		return err
	}

	l.l.Debug("Attached tracepoint", log.String("group", group), log.String("name", name))
	l.tracepoints = append(l.tracepoints, tracepoint)
	return nil
}

// See https://github.com/iovisor/bcc/pull/3315
func findKernelSymbolsByRegex(regex string) ([]string, error) {
	resolver, err := kallsyms.DefaultKallsymsResolver()
	if err != nil {
		return nil, err
	}
	symbols, err := resolver.LookupSymbolRegex(regex)
	if err != nil {
		return nil, fmt.Errorf("failed to lookup kprobe symbol by regex %s: %w", regex, err)
	}

	return symbols, nil
}

func (l *Links) Kprobe(symbolRegex string, prog *ebpf.Program) error {
	symbols, err := findKernelSymbolsByRegex(symbolRegex)
	if err != nil {
		return err
	}

	var errs []error
	for _, symbol := range symbols {
		kprobe := &kprobe{config: kprobeConfig{symbol: symbol}}
		err = kprobe.create(prog)
		if err == nil {
			l.kprobes = append(l.kprobes, kprobe)
			l.l.Debug("Attached kprobe", log.String("symbol", symbol))
			return nil
		}

		errs = append(errs, err)
	}

	return fmt.Errorf("failed to attach kprobe by regex %s: %w", symbolRegex, errors.Join(errs...))
}

type reloadable interface {
	idempotentCloser
	create(prog *ebpf.Program) error
}

func reloadLinks[T reloadable](l log.Logger, links []T, prog *ebpf.Program) (err error) {
	err = closeLinks(links)
	if err != nil {
		return err
	}

	defer func() {
		if err != nil {
			closeErr := closeLinks(links)
			if closeErr != nil {
				l.Warn("Failed to close links", log.Error(closeErr))
			}
		}
	}()

	for _, link := range links {
		err = link.create(prog)
		if err != nil {
			return err
		}
	}

	return nil
}

func (l *Links) Reload(tracepointProg *ebpf.Program, kprobeProg *ebpf.Program) error {
	return errors.Join(
		reloadLinks(l.l, l.tracepoints, tracepointProg),
		reloadLinks(l.l, l.kprobes, kprobeProg),
	)
}
