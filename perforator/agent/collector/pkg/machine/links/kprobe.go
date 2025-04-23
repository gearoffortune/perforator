package links

import (
	"fmt"

	"github.com/cilium/ebpf"
	"github.com/cilium/ebpf/link"
)

type kprobeConfig struct {
	symbol string
}

type kprobe struct {
	link.Link
	config kprobeConfig
}

func (k *kprobe) close() error {
	if k.Link == nil {
		return nil
	}

	err := k.Link.Close()
	k.Link = nil

	return err
}

func (k *kprobe) create(prog *ebpf.Program) (err error) {
	k.Link, err = link.Kprobe(k.config.symbol, prog, nil)
	if err != nil {
		err = fmt.Errorf("failed to attach kprobe %s: %w", k.config.symbol, err)
		return
	}

	return
}
