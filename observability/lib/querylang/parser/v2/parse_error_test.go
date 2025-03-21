package parserv2_test

import (
	"fmt"
	"testing"

	"github.com/stretchr/testify/assert"
	"github.com/stretchr/testify/require"

	parserv2 "github.com/yandex/perforator/observability/lib/querylang/parser/v2"
)

func TestNewParseError(t *testing.T) {
	t.Run("return nil if no errors", func(t *testing.T) {
		err := parserv2.NewParseError(make([]error, 0), make([]error, 0))

		require.NoError(t, err)
	})

	t.Run("return syntax error", func(t *testing.T) {
		syntaxErr := fmt.Errorf("cannot parse")
		err := parserv2.NewParseError([]error{syntaxErr}, make([]error, 0))

		require.Error(t, err)
		assert.EqualError(t, err, "syntax errors: cannot parse")
	})

	t.Run("return semantic error", func(t *testing.T) {
		semanticErr := fmt.Errorf("cannot parse")

		err := parserv2.NewParseError(make([]error, 0), []error{semanticErr})

		require.Error(t, err)
		assert.EqualError(t, err, "semantic errors: cannot parse")
	})

	t.Run("return syntax error even if there is semantic error", func(t *testing.T) {
		syntaxErr := fmt.Errorf("cannot parse")
		semanticErr := fmt.Errorf("cannot parse")

		err := parserv2.NewParseError([]error{syntaxErr}, []error{semanticErr})

		require.Error(t, err)
		assert.EqualError(t, err, "syntax errors: cannot parse")
	})
}
