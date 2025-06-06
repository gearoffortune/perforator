package postgrestaskservice

import (
	"time"

	"github.com/yandex/perforator/perforator/internal/asynctask"
)

const (
	defaultTasksTableName = "tasks"
	agentTasksTableName   = "agent_relayer_tasks"
)

type Config struct {
	PingPeriod  time.Duration `yaml:"ping_period"`
	PingTimeout time.Duration `yaml:"ping_timeout"`
	MaxAttempts int           `yaml:"max_attempts"`
}

func namespaceToTableName(namespace asynctask.Namespace) string {
	switch namespace {
	case asynctask.NamespaceAgent:
		return agentTasksTableName
	default:
		return defaultTasksTableName
	}
}

func (c *Config) fillDefault() {
	if c.PingPeriod == 0 {
		c.PingPeriod = time.Second
	}
	if c.PingTimeout == 0 {
		c.PingTimeout = time.Second * 10
	}
	if c.MaxAttempts == 0 {
		c.MaxAttempts = 3
	}
}
