package links

import (
	"fmt"

	"github.com/cilium/ebpf"
	"github.com/cilium/ebpf/link"
)

type tracepointConfig struct {
	group string
	name  string
}

type tracepoint struct {
	link.Link
	config tracepointConfig
}

func (t *tracepoint) close() error {
	if t.Link == nil {
		return nil
	}

	err := t.Link.Close()
	t.Link = nil

	return err
}

func (t *tracepoint) create(prog *ebpf.Program) (err error) {
	t.Link, err = link.Tracepoint(t.config.group, t.config.name, prog, nil)
	if err != nil {
		err = fmt.Errorf("failed to attach tracepoint %s %s: %w", t.config.group, t.config.name, err)
		return
	}

	return
}
